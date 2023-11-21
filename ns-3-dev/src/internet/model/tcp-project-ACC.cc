/*
 * Copyright (c) 2019 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Apoorva Bhargava <apoorvabhargava13@gmail.com>
 *         Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 *
 */

#include "tcp-project-ACC.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>

// True if in training 
int training = 0;

// Number of actions
const int numActions = 4; // Assuming 3 possible values for the action

// Number of states 
const int numStates = 8;

// Exploration rate (epsilon)
double epsilon = 0.8;

// Initialize Q-table
std::vector<std::vector<double>> qTable(numStates, std::vector<double>(numActions, 0.0));

// Variables to store previous state and action
int prevState = 0;
int prevAction = 0;
double prev_RTT;
int prev_cwnd;

int iteration_count = 0;


namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TcpProjectACC");
NS_OBJECT_ENSURE_REGISTERED(TcpProjectACC);

TypeId
TcpProjectACC::GetTypeId()
{
    static TypeId tid = TypeId("ns3::TcpProjectACC")
                            .SetParent<TcpCongestionOps>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpProjectACC>();
    return tid;
}

TcpProjectACC::TcpProjectACC()
    : TcpCongestionOps()
{
    NS_LOG_FUNCTION(this);
}

TcpProjectACC::TcpProjectACC(const TcpProjectACC& sock)
    : TcpCongestionOps(sock)
{
    NS_LOG_FUNCTION(this);
}

TcpProjectACC::~TcpProjectACC()
{
}

// Function to select a state given some state parameters
int selectState(int current_cwnd, double current_RTT){
    
    if (current_cwnd < 20000)
        return 0;
    else if (current_cwnd >= 20000 && current_cwnd < 30000)
        return 1;
    else if (current_cwnd >= 30000 && current_cwnd < 40000)
        return 2;
    else if (current_cwnd >= 40000 && current_cwnd < 45000)
        return 3;
    else if (current_cwnd >= 45000 && current_cwnd < 50000)
        return 4;
    else if (current_cwnd >= 50000 && current_cwnd < 55000)
        return 5;
    else if (current_cwnd >= 55000 && current_cwnd < 60000)
        return 6;
    else
        return 7;
}

// Function to select an action with epsilon-greedy strategy
int selectAction(int state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    if (dis(gen) < epsilon) {
        // Explore: Choose a random action
        return rand() % numActions;
    } else {
        // Exploit: Choose the action with the highest Q-value
        int bestAction = 0;
        for (int action = 1; action < numActions; action++) {
            if (qTable[state][action] > qTable[state][bestAction]) {
                bestAction = action;
            }
        }
        return bestAction;
    }
}

// Function to update Q-values
std::vector<std::vector<double>> updateQValue(std::vector<std::vector<double>> qTable, int prevState, int prevAction, double reward) {
    qTable[prevState][prevAction] = reward;
    return qTable;
}

// Function to calculate reward based on the next state
double calculateReward(int prev_cwnd, int current_cwnd) {
    return current_cwnd - prev_cwnd; 
}

// Function to select the desired action
int perform_action(int action, int current_cwnd, int iteration_count){
    switch (action)
    {
    case 0:
        return current_cwnd;
    case 1:
        return current_cwnd + 1;
    case 2:
        return current_cwnd + 1000; // mettere rand 500-1000?
    case 3:
        return current_cwnd * (1 + 0.5);
    default:
        return current_cwnd;
        std::cout << "You are in Default" << std::endl;
    }
    
}

void print_table(std::vector<std::vector<double>> qTable, int numActions, int numStates, int iteration_count){
    // std::cout << "Iteration: " << iteration_count << std::endl;
    //     for (int i = 0; i < numStates; i++) {
    //         for (int j = 0; j < numActions; j++) {
    //             std::cout << qTable[i][j] << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    //     std::cout << "------------------------" << std::endl;
    
    // Open a file for writing
        std::ofstream outputFile("qTableMyTCP.txt");

        // Check if the file is opened successfully
        if (!outputFile.is_open()) {
            std::cerr << "Unable to open the file." << std::endl;
        }
        for (int i = 0; i < numStates; i++) {
            for (int j = 0; j < numActions; j++) {
                outputFile << qTable[i][j] << " ";
            }
            outputFile << std::endl;
        }

        // Close the file
        outputFile.close();
}

// Function to load Q-values
std::vector<std::vector<double>> loadQValue(std::vector<std::vector<double>> qTable) {

    // Open the file for reading
    std::ifstream inputFile("qTableMyTCP.txt");

    // Check if the file is opened successfully
    if (!inputFile.is_open()) {
        std::cerr << "Unable to open the file." << std::endl;
    }
    // Read the content from the file and populate the vector
    for (int i = 0; i < numStates; ++i) {
        std::string line;
        if (std::getline(inputFile, line)) {
            std::istringstream iss(line);
            for (int j = 0; j < numActions; ++j) {
                if (!(iss >> qTable[i][j])) {
                    std::cerr << "Error reading data from the file." << std::endl;
                }
            }
        } else {
            std::cerr << "Error reading line from the file." << std::endl;
        }
    }

    // Close the file
    inputFile.close();

    print_table(qTable, numActions, numStates, iteration_count);

    return qTable;
}

void
TcpProjectACC::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);
    
    double current_RTT = tcb->m_lastRtt.Get().GetDouble();
    int current_cwnd = tcb -> m_cWnd;

    if (current_cwnd >= 60000)
        current_cwnd = current_cwnd/2;
        tcb -> m_cWnd = current_cwnd;

    int currentState; // Initial state, both variables at 0
    int action;

    // Select Current State based on Congestion Window Size and last RTT
    currentState = selectState(current_cwnd, current_RTT);

    if (iteration_count > 1 && training == 1){
        // Calculate reward based on the next state
        double reward = calculateReward(prev_cwnd, current_cwnd);

        // Update Q-value for the previous state-action pair
        qTable = updateQValue(qTable, prevState, prevAction, reward);
    }
    else if (iteration_count == 0 && training == 0){
        qTable = loadQValue(qTable);
        epsilon = 0;
    }

    // Select an action using epsilon-greedy strategy
    action = selectAction(currentState);

    tcb -> m_cWnd = perform_action(action, current_cwnd, iteration_count);
    
    // Store the current state and action as previous for the next iteration
    prevState = currentState;
    prevAction = action;
    prev_RTT = current_RTT;
    prev_cwnd = current_cwnd;

    // Print the Q-table at some intervals if needed
    if (iteration_count % 1000 == 0 && training == 1) {
        // After training, you can use the Q-table to make decisions about actions.
        print_table (qTable, numActions, numStates, iteration_count);
    }

    iteration_count += 1;
}

std::string
TcpProjectACC::GetName() const
{
    return "TcpProjectACC";
}

uint32_t
TcpProjectACC::GetSsThresh(Ptr<const TcpSocketState> state, uint32_t bytesInFlight)
{
    NS_LOG_FUNCTION(this << state << bytesInFlight);

    // In Linux, it is written as:  return max(tp->snd_cwnd >> 1U, 2U);
    return std::max<uint32_t>(2 * state->m_segmentSize, state->m_cWnd / 2);
}

Ptr<TcpCongestionOps>
TcpProjectACC::Fork()
{
    return CopyObject<TcpProjectACC>(this);
}

} // namespace ns3
