#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include <thread>
#include <fdsc_utils/lowCmd.hpp>
#include <fdsc_utils/lowState.hpp>
// #include <fdsc_utils/unitreeConnect.hpp>
// #include <fdsc_utils/unitreeConnectBoostAsyc.hpp>
#include <fdsc_utils/unitreeConnectBoost.hpp> //more fast and safe than the above one
#include <fdsc_utils/complex.hpp>
#include <fdsc_utils/color.hpp>
#include <algorithm>
void show_joint_info(const std::vector<FDSC::MotorState> & mobj)
{
    for (int i = 0; i < 12; i++)
    {
        std::cout<<"Joint: "<<i<<"   Motor Mode: "<< mobj[i].mode <<" Motor NTC: "<<mobj[i].temperature<<" ";
        std::cout<<" q: "<<mobj[i].q<<" dq: "<<mobj[i].dq<<" ddq: "<<mobj[i].ddq<<" TauEst: "<<mobj[i].tauEst<<std::endl;
    }

}
void show_info(const FDSC::lowState & lstate)
{

        std::cout << "=============================" << std::endl;
        std::cout << SetForeGRN <<"------------------HardSoft Version info: -------------------" << std::endl;
        std::cout << "SN: " << "\t";     FDSC::byte_print(lstate.SN,false);std::cout << FDSC::decode_sn(lstate.SN)<< std::endl;
        std::cout << "Version: " << "\t";FDSC::byte_print(lstate.version,false);std::cout<< FDSC::decode_version(lstate.version) << std::endl;
        std::cout << "Bandwidth: " << "\t"<<FDSC::hex_to_kp_kd(lstate.bandWidth)<< std::endl;
        std::cout << SetForeGRN <<"------------------IMU info: -------------------" << std::endl;
        std::cout << "IMU Quaternion: ";    FDSC::pretty_show_vector(lstate.imu_quaternion);std::cout << std::endl;
        std::cout << "IMU Gyroscope: ";     FDSC::pretty_show_vector(lstate.imu_gyroscope)  ;std::cout<< std::endl;
        std::cout << "IMU Accelerometer: "; FDSC::pretty_show_vector(lstate.imu_accelerometer);std::cout << std::endl;
        std::cout << "IMU RPY: ";           FDSC::pretty_show_vector(lstate.imu_rpy) ;
        std::cout<<  "IMU Temperature: " << float(lstate.temperature_imu) << std::endl;
        // std::vector<uint8_t> imu_data(data.begin()+22,data.begin()+75);
        // FDSC::show_byte_data(data,8);
        // std::cout<<std::endl;
        std::cout<<SetForeGRN << "------------------Joint info: -------------------" << std::endl;
        show_joint_info(lstate.motorstate);
        // BMS do not have any data from UDP data
        std::cout<<SetForeRED << "------------------BMS info(No data): -------------------" << std::endl;
        std::cout << "SOC: " << int(lstate.SOC) << std::endl;
        // std::cout << "Overall Voltage: " << FDSC::getVoltage(lstate.cell_vol) << "mV" << std::endl;
        std::cout << "Current: " << lstate.current << "mA" << std::endl;
        std::cout << "Cycles: " << lstate.cycle << std::endl;
        std::cout << "Temps BQ: " << lstate.BQ_NTC[0] << "°C, " << lstate.BQ_NTC[1] << "°C" << std::endl;
        std::cout << "Temps MCU: " << lstate.MCU_NTC[0] << "°C, " << lstate.MCU_NTC[1] << "°C" << std::endl;
        std::cout<<SetForeRED << "------------------Foot Force info(No data In Go1 Air): -------------------" << std::endl;
        std::cout<<"Footforce: ";
        for (auto data_f : lstate.footForce)
        {
            std::cout<<float(data_f)<<" ";
        }
        std::cout<<std::endl;
        std::cout<<"FootforceEst: ";
        for (auto data_f : lstate.footForceEst)
        {
            std::cout<<float(data_f)<<" ";
        }
        std::cout << "=============================" << std::endl;
        
}

int main() {
    std::string settings = "LOW_WIRED_DEFAULTS";//"SIM_DEFAULTS";//HIGH_WIFI_DEFAULTS
    FDSC::UnitreeConnection conn(settings);
    conn.startRecv();


    FDSC::lowCmd lcmd;
    FDSC::lowState lstate;
    FDSC::MotorCmdArray mCmdArr;
    // Send an empty command to tell the dog the receive port and initialize the connection
    std::vector<uint8_t> cmd_bytes = lcmd.buildCmd(false);

    conn.send(cmd_bytes);
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Some time to collect packets

    int rate_count = 0;

    float torque = 0.0f;
    int motiontime = 0;
    int max_iter = 240;
    FDSC::show_in_lowcmd();
    bool show_data = false;
    while (true) {
       
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        motiontime++;
        std::vector<std::vector<uint8_t>> dataall;// = conn.getData();
        conn.getData(dataall);
        if (dataall.size()!=0)
        {
            std::vector<uint8_t> data = dataall.at(dataall.size()-1);
            lstate.parseData(data);
            if (show_data && motiontime % 100 ==0)
            {
                std::cout<<"reveive pkg len: "<<dataall.size()<<std::endl;
                std::cout<<"loop: "<<motiontime<<" datalen: " <<std::dec<<data.size()<<std::endl;
                show_info(lstate);
            }
            if (motiontime > 0) {

                if (motiontime >= 500) {
                    torque = (0.0f-lstate.motorstate[FDSC::jointMapping["FR_1"]].q)*10.0f + (0.0f - lstate.motorstate[FDSC::jointMapping["FR_1"]].dq)*1.0f;
                    torque = std::min(std::max(torque,-5.0f),5.0f);
                    // MotorModeLow m, q_,dq_,tau_,kp_, kd_
                    std::vector<float> FR1_joint{0.0f, 0.0f,torque, 0.0f, 1.0f};
                    mCmdArr.setMotorCmd("FR_1", FDSC::MotorModeLow::Servo, FR1_joint);
                    lcmd.motorCmd = mCmdArr;
                }

                // std::cout<<SetForeMAG<<"Motiondata: "<<motiontime<<" speed "<<speed<<" real_dq: "<<lstate.motorstate[FDSC::jointMapping["FL_2"]].dq<<std::endl;
                std::vector<uint8_t> cmdBytes = lcmd.buildCmd(false);
                conn.send(cmdBytes);
            }
        }else{
            std::cout<<"out:motiontime: "<<motiontime<<" datasize: "<<dataall.size()<<std::endl;
        }
    
    }

    return 0;
}
