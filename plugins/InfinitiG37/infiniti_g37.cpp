#include "infiniti_g37.hpp"

Theme* InfinitiG37::theme = nullptr;
Climate* InfinitiG37::climate = nullptr;
bool InfinitiG37::duelClimate = false;


bool InfinitiG37::init(ICANBus* canbus){
    this->duelClimate=false;
    this->theme = Theme::get_instance();
    this->climate = new Climate();
    this->climate->set_max_fan_speed(7);
    this->climate->setProperty("tab_title", "Climate");
    std::function<void(QByteArray)> headlightCallback = InfinitiG37::monitorHeadlightStatus;
    canbus->registerFrameHandler(0x60D, headlightCallback);
    std::function<void(QByteArray)> climateControlCallback = InfinitiG37::updateClimateDisplay;
    canbus->registerFrameHandler(0x54B, climateControlCallback);
    std::function<void(QByteArray)> temperatureControlCallback = InfinitiG37::updateTemperatureDisplay;
    canbus->registerFrameHandler(0x542, temperatureControlCallback);
    

    G37_LOG(info)<<"loaded successfully";
    return true;
}

// 60D
// FIRST BYTE:
//     headlights:
//         08 = OFF
//         0E = AUTO (ON)
//         0C = SIDES
//         0E = ON
// SECOND BYTE:
//     foglights:
//         06 = OFF
//         07 = ON

void InfinitiG37::monitorHeadlightStatus(QByteArray payload){
    if(payload.at(0) == 0x0E){
        //headlights are ON - turn to dark mode
        if(theme->get_mode() != true){
            theme->set_mode(true);
        }
    }
    else{
        //headlights are off or not fully on - make sure is light mode
        if(theme->get_mode() != false){
            theme->set_mode(false);
        }
    }
}

// 54B
// 54B  03 08 80 02 04 00 00 01 


// 1st byte 
//  AC status
//  0x03: OFF
//  0x02 ON - cold?
// 3rd byte:
//   mode:
//    A0 defrost+leg
//    88 head
//    90 head+feet
//    98 feet
//    A8 defrost
// 4th byte:
//  duel climate:
//      61: on & recirculation?
//      01: off & recirculation
//      62: on
//      02: off
//      
// 5th byte:
//    fan:
//     04 off
//     0C 1
//     14 2
//     1C 3
//     24 4
//     2C 5
//     34 6
//     3C 7
//
int oldPassTemp = 0;
int oldDriverTemp= 0;
int oldAirflow = 0;
int oldFan = 0;
int oldStatus =0;

void InfinitiG37::updateClimateDisplay(QByteArray payload){
    if(payload.at(3)==0x62 || payload.at(3) == 0x61){
        duelClimate = true;
    }
    else{
        duelClimate = false;
    }
    if(payload.at(0) != oldStatus){
        oldStatus = payload.at(0);
        if(payload.at(0)==0x03){
            climate->set_airflow(0);
            climate->set_fan_speed(0);
            G37_LOG(info)<<"Climate is off";
            return;
        }
    }
    if((unsigned char)payload.at(2)!=oldAirflow){
        G37_LOG(info)<<"2: "<<(int)(unsigned char)payload.at(2);
        G37_LOG(info)<<"OLD: "<<oldAirflow;

        switch((unsigned char)payload.at(2)){
            case(0xA0):
                climate->set_airflow(0b101);
                break;
            case(0x88):
                climate->set_airflow(0b010);
                break;
            case(0x90):
                climate->set_airflow(0b110);
                break;
            case(0x98):
                climate->set_airflow(0b100);
                break;
            case(0xA8):
                climate->set_airflow(0b1);
                break;
        }
        oldAirflow = (unsigned char)payload.at(2);
    }
    if((unsigned char)payload.at(4)!=oldFan){
        switch((unsigned char)payload.at(4)){
            case(0x04):
                climate->set_fan_speed(0);
                break;
            case(0x0C):
                climate->set_fan_speed(1);
                break;
            case(0x14):
                climate->set_fan_speed(2);
                break;
            case(0x1C):
                climate->set_fan_speed(3);
                break;
            case(0x24):
                climate->set_fan_speed(4);
                break;
            case(0x2C):
                climate->set_fan_speed(5);
                break;
            case(0x34):
                climate->set_fan_speed(6);
                break;
            case(0x3C):
                climate->set_fan_speed(7);
                break;
        }
        oldFan = (unsigned char)payload.at(4);
    }
}

// 542  00 5A 00 00 00 00 00 01 | 90F
// 542  00 3C 00 00 00 00 00 01 | 60F
// SECOND BYTE:
//     temperature goal:
//         1:1 to temperature in F
// THIRD BYTE:
//      duel climate temp goal
void InfinitiG37::updateTemperatureDisplay(QByteArray payload){
    if(oldDriverTemp!=(unsigned char)payload.at(1)){
        climate->set_driver_temp((unsigned char)payload.at(1));
        oldDriverTemp = (unsigned char)payload.at(1);
    }
    if(duelClimate){
        if(oldPassTemp!=(unsigned char)payload.at(2)){
            climate->set_passenger_temp((unsigned char)payload.at(2));
            oldPassTemp = (unsigned char)payload.at(2);
        }
    }else{
        if(oldPassTemp!=(unsigned char)payload.at(1)){
            climate->set_passenger_temp((unsigned char)payload.at(1));
            oldPassTemp=(unsigned char)payload.at(1);

        }
    }
}


QList<QWidget *> InfinitiG37::tabs()
{
    QList<QWidget *> tabs;
    tabs.append(this->climate);
    return tabs;
}
