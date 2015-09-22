#pragma once

#include <ctime>		//Include NULL
#include <iostream>		//Include rand()
#include <vector>
#include <map>
#include<sstream>
#include "car.h"

class Queue {
public:
	//### Constructors & Destructors ###
	Queue();
	Queue(int ID, double _maxLength, double _distanceBetweenCars, std::map<int,double> _capacities, std::map<int,double> _greenTime, std::map<int,double> _cycle, std::map<int,double> _offset);
	Queue(Json::Value value);
	~Queue();

	//### Getters ###
	int ID();
	double length();
	std::vector<Car>& getQueue();
	std::map<int, double> getCapacities();
	std::map<int,double> getGreenTime();
	std::map<int,double> getCycle();
	std::map<int,double> getOffset();

	//### Dynamic methods ###
	int weight(int nextNode);
	void iterCarsProg();
	void addCar(Car C);
	void removeCar();
	void moveFakeCars(int timestep);
	std::map<int,double> getRealCapacity(int time);
	double getMinCapacity();

	//### Serialization ###

	Json::Value toJson();
	bool operator==(const Queue & q) const;

	
private:
	//### Constant parameters ###
	int queueID;
	double maxLength;
	double distanceBetweenCars;
	std::vector<int> nextNodes;
	std::map<int, double> capacities;		//int of next node && capacity value to this next node

	std::map<int,double> greenTime;
	std::map<int,double> cycle;
	std::map<int,double> offset;

	//### Cars in queue ###
	std::vector<Car> cars;

	//### Outputs ###
	std::vector<double> lengthOverTime;


};	 