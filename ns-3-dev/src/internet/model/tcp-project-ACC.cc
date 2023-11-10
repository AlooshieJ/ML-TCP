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

// ciao

#include "tcp-project-ACC.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include <iostream>
#include <vector>
#include <random>
#include <cmath>

// Number of actions
const int numActions = 10; // Assuming 10 possible values for the action

// Number of states 
const int numStates = 10;

// Learning rate (alpha)
const double learningRate = 0.1;

// Discount factor (gamma)
const double discountFactor = 0.9;

// Exploration rate (epsilon)
const double epsilon = 0.2;

// Initialize Q-table
std::vector<std::vector<double>> qTable(numStates, std::vector<double>(numActions, 0.0));

// Variables to store previous state and action
int prevState = 0;
int prevAction = 0;
double prev_RTT;
int prev_cwnd;


int vector_sizes = 5;
int iteration_count = 0;
// Vector to store past RTT values
std::vector<double> RTT_in_time(vector_sizes, 0.0);
// Vector to store past cwind values
std::vector<double> cwind_in_time(vector_sizes, 0.0);

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

std::vector<double> leftShiftArray(std::vector<double> arr) {
    for (int j = 0; j < vector_sizes - 1; j++) {
        arr[j] = arr[j + 1]; // Shift each element one position to the left
    }
    return arr;
}

// Function to select a state given some state parameters
int selectState(int current_cwnd, double current_RTT){
    
    return 1;
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
    double bestNextAction = qTable[prevState][selectAction(prevState)];
    qTable[prevState][prevAction] += learningRate * (reward + discountFactor * bestNextAction - qTable[prevState][prevAction]);
    return qTable;
}

// Function to calculate reward based on the next state
double calculateReward(double prev_RTT, double current_RTT) {
    // Your reward calculation logic based on the next state

    return prev_RTT - current_RTT; // Example to change
}

// Function to select the desired action
int perform_action(int action, int current_cwnd){
    return 1000;
}

void print_table(std::vector<std::vector<double>> qTable, int numActions, int numStates, int iteration_count){
    std::cout << "Iteration: " << iteration_count << std::endl;
        for (int i = 0; i < numStates; i++) {
            for (int j = 0; j < numActions; j++) {
                std::cout << qTable[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "------------------------" << std::endl;
}

void
TcpProjectACC::IncreaseWindow(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);
    
    double current_RTT = tcb->m_lastRtt.Get().GetDouble();
    int current_cwnd = tcb -> m_cWnd;


    int currentState; // Initial state, both variables at 0
    int action;

    // Select Current State based on Congestion Window Size and last RTT
    currentState = selectState(current_cwnd, current_RTT);

    if (iteration_count > 1){
        // Calculate reward based on the next state
        double reward = calculateReward(prev_RTT, current_RTT);

        // Update Q-value for the previous state-action pair
        qTable = updateQValue(qTable, prevState, prevAction, reward);
    }

    // Select an action using epsilon-greedy strategy
    action = selectAction(currentState);

    tcb -> m_cWnd = perform_action(action, current_cwnd);
   
    // Store the current state and action as previous for the next iteration
    prevState = currentState;
    prevAction = action;
    prev_RTT = current_RTT;
    prev_cwnd = current_cwnd;

    // Print the Q-table at some intervals if needed
    if (iteration_count % 100 == 0) {
        print_table (qTable, numActions, numStates, iteration_count);
    }

    // After training, you can use the Q-table to make decisions about actions.

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
