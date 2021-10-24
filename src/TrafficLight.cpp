#include <iostream>
#include <random>
#include <cstdlib>
#include <thread>

#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);

    // wait for new message
    _condition.wait(uLock, [this] {return !_queue.empty();});

    //pull new message from the queue and remotee it from the queue
    T msg = std::move(_queue.back());
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // protect the queue 
    std::lock_guard<std::mutex> lck(_mutex);
    
    _queue.clear();
    _queue.emplace_back(std::move(msg));

    _condition.notify_one();
}


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{

}

void TrafficLight::waitForGreen()
{

    while(true) {

        //check the queue for a green traffic light
        if (_msgQueue.receive() == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.push_back(std::thread(&TrafficLight::cycleThroughPhases, this));    
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // random number generator
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<double> dist(4000, 6000);

    double timeLimit = dist(mt);
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = std::chrono::steady_clock::now();
    auto duration = -1;

    while(true) {
        endTime = std::chrono::steady_clock::now();
        duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        if (duration >= timeLimit) {

            // toggle the current phase between red and green
            if (_currentPhase == TrafficLightPhase::green) { 
                _currentPhase = TrafficLightPhase::red;
            }
            else {
                _currentPhase = TrafficLightPhase::green;
            }

             // update msQueue
            _msgQueue.send(std::move(_currentPhase));

            // new random number
            timeLimit = dist(mt);

            // reset timer
            startTime = std::chrono::steady_clock::now();
        }
       
        // wait before next toggle
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
