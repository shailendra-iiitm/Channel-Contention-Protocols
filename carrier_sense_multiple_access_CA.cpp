#include <bits/stdc++.h>
#define MaxPacketLength 10
#define MaxBackoffTime  8
#define NumSlots 50

using namespace std;

vector<int> successfulTransmissions;
int Totalcollisions = 0;

class Channel{
public:
    set<int> DevicesOnChannel = {};

    bool isChannelFree(){
        return DevicesOnChannel.size() == 0;
    }

    bool isChannelBusy(){
        return !isChannelFree();
    }

    void clear(){
        DevicesOnChannel.clear();
    }
};

class Device{
public:
    int Device_id;
    float DataSendingTime;
    float Backofftime;
    float Timespent;
    bool Transmitting;
    int BackoffAttempts;
    int Num_Collisions;

    Device(int Device_id, float DataSendingTime) : Device_id(Device_id), DataSendingTime(DataSendingTime), Backofftime(0), Timespent(0), Transmitting(false), BackoffAttempts(0), Num_Collisions(0) {}

    void backoff(){
        Timespent = 0;
        Backofftime = (rand() % MaxBackoffTime) + 2;
        cout << "Device " << Device_id + 1 << " is backed off by " << Backofftime << " Seconds" << endl;
        BackoffAttempts++;
    }

    void collide(){
        cout << "Device " << Device_id + 1 << " collided with other nodes." << endl;
        Num_Collisions++;
    }

    bool attemptTransmission(Channel &channel){
        if (channel.DevicesOnChannel.size() == 1 && Device_id == *channel.DevicesOnChannel.begin()){
            return true;
        }

        if (Timespent == DataSendingTime){
            return false;
        }

        if (channel.isChannelBusy()){
            backoff();
            return false;
        }
        return true;
    }

    void transmit(Channel &channel){
        channel.DevicesOnChannel.insert(Device_id);
        Timespent++;
    }

    bool donePartially(){
        return Timespent < DataSendingTime && Timespent > 0;
    }

    void decrementBackoff(){
        if (Backofftime > 0){
            Backofftime--;
        }
    }
};

int main(){
    srand(time(0));

    int numDevices;
    Channel oldchannel, channel;
    vector<Device> Devices;

    cout << "Number of Devices on the network: ";
    cin >> numDevices;

    cout << "DATA SENDING TIMES GENERATED AUTOMATICALLY FOR THE DEVICES ARE AS:" << endl;
    for (int i = 0; i < numDevices; i++){
        int DataSendingTime = (rand() % MaxPacketLength) + 1;
        cout << "Device No." << i + 1 << ": " << DataSendingTime << endl;
        Devices.push_back(Device(i, DataSendingTime));
    }

    for (int slot = 0; slot < NumSlots; slot++){
        cout << "Time " << slot + 1 << ":" << endl;

        set<int> oldChannel = oldchannel.DevicesOnChannel;

        for (auto &Device : Devices){
            for (auto &Device_id : oldChannel){
                if (Device_id == Device.Device_id && !Device.donePartially()){
                    if (oldchannel.DevicesOnChannel.find(Device_id) != oldchannel.DevicesOnChannel.end()){
                        oldchannel.DevicesOnChannel.erase(oldchannel.DevicesOnChannel.find(Device.Device_id));
                        successfulTransmissions.push_back(Device_id);
                    }
                }
            }
        }

        if (successfulTransmissions.size() == numDevices){
            cout << "\nAll devices have finished transmitting.\n";
            break;
        }

        cout << "Devices from previous transition still continuing: ";
        for (auto &Device : oldchannel.DevicesOnChannel){
            cout << Device << ",";
        }
        cout << endl;

        for (auto &Device : Devices){
            if (Device.Backofftime == 0){
                if (Device.attemptTransmission(oldchannel)){
                    Device.transmit(channel);
                }
            }
        }

        cout << "Devices of current transition: ";
        for (auto &Device : channel.DevicesOnChannel){
            cout << Device << ",";
        }
        cout << endl;

        if (channel.DevicesOnChannel.size() > 1){
            for (auto &Device : Devices){
                for (auto &Device_id : channel.DevicesOnChannel){
                    if (Device_id == Device.Device_id){
                        Device.collide();
                        Device.backoff();
                    }
                }
            }
            Totalcollisions++;
            channel.clear();
        }

        oldchannel = channel;
        channel.clear();

        for (auto &Device : Devices){
            Device.decrementBackoff();
        }
    }

    cout << "Number of Devices which have Successfully Transmitted entire message: " << successfulTransmissions.size() << endl;

    cout << "And the Devices are:  ";
    sort(successfulTransmissions.begin(), successfulTransmissions.end());
    for (auto &x : successfulTransmissions){
        cout << x << " ";
    }
    if (successfulTransmissions.size() == 0){
        cout << "None";
    }
    cout << endl;

    cout << "Total Number of Collisions: " << Totalcollisions << endl;
    cout << "Backoff Attempts: " << endl;
    int sum = 0;
    for (auto &Device : Devices){
        cout << "Node " << Device.Device_id << ": " << Device.BackoffAttempts << endl;
        sum += Device.BackoffAttempts;
    }
    cout << "Total Number of Backoff Attempts: " << sum << endl << endl;

    return 0;
}
