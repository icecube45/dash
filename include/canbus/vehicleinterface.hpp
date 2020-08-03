#pragma once
#include "socketcanbus.hpp"

class VehicleInterface
{
public:
    virtual ~VehicleInterface() = default;
    virtual bool init(SocketCANBus* canbus) = 0;
};

#define VehicleInterface_iid "org.dash.canbus.VehicleInterface"

Q_DECLARE_INTERFACE(VehicleInterface, VehicleInterface_iid)