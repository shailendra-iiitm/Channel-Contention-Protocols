

#include <bits/stdc++.h>
#define maxBackoff 15            //maximum backoff interval
#define numSlots 60              //number of time slots for demonstration
#define maxPacketLength 4        //maximum possible packet length
#define snrThreshold 23.53         // SNR threshold for successful transmission

using namespace std;

//vector to store nodes which have done transmitting
vector<int> successfulTransmissions;
vector<int> blockedNodes;
vector<int> failedDueToSNR;
int Totalcollisions = 0;

// Channel has channelNodes which are currently using the channel
class Channel {
public:
    set<int> channelNodes = {};

    bool isIdle() {
        return channelNodes.size() == 0;
    }

    bool isBusy() {
        return !isIdle();
    }

    void clear() {
        channelNodes = {};
    }
};

// Function to calculate SNR based on distance
double calculateSNR(double distance, double Pt_dBm, double NoisePower_dBm) {
    // Simplified path loss model (free-space path loss)
    double pathLoss_dB = 20 * log10(distance) + 20 * log10(2.4 * pow(10, 9)) - 147.55;
    double receivedPower_dBm = Pt_dBm - pathLoss_dB;

    // Calculate SNR
    double SNR_dB = receivedPower_dBm - NoisePower_dBm;
    
    
    
    
    return SNR_dB;
}

// Node class definition
class Node {
public:
    int id;                     // node identifier
    int dataSendingTime;        // time required to send data
    double distance;            // distance between device and receiver
    int backoff;                // remaining waiting time
    bool transmitting;          // true if currently transmitting data
    int timesent;               // time spent sending the data
    int backoffAttempts;        // number of attempts
    int collisions;             // number of collisions experienced
    double snr;                 // SNR value for the node
    bool transmitted;           // indicates if the node has successfully transmitted

    // Constructor
    Node(int id, int dataSendingTime, double distance, double Pt_dBm, double NoisePower_dBm)
        : id(id), dataSendingTime(dataSendingTime), distance(distance), backoff(0), transmitting(false), timesent(0), backoffAttempts(0), collisions(0), transmitted(false) {
        snr = calculateSNR(distance, Pt_dBm, NoisePower_dBm);  // Calculate SNR based on distance
    }

    // Generate backoff value randomly
    void Backoff() {
        timesent = 0;
        backoff = rand() % maxBackoff + 5;
        cout << "Node " << id << " backed off by " << backoff << endl;
        backoffAttempts++;
    }

    void Collide() {
        cout << "Node " << id << " collided with other nodes and ";
        collisions++;
    }

    // Check if the node is done transmitting
    bool isDone() {
        return timesent == dataSendingTime;
    }

    // Add this node to the channel
    void transmit(Channel &channel) {
        channel.channelNodes.insert(id);
        timesent++;
    }

    // Check whether it is partially done or not
    bool donePartially() {
        return timesent < dataSendingTime && timesent > 0;
    }

    void decrementBackoff() {
        if (backoff > 0) {
            backoff--;
        }
    }
};

int main() {
    srand(time(0));   // Seeding with current timestamp to generate different values when it runs again

    int numNodes;
    Channel oldchannel, channel;  // oldchannel corresponds to the channel in the previous time slot
    vector<Node> nodes;

    cout << "Number of Nodes in network: ";
    cin >> numNodes;

    cout << "Enter y if you want to manually give data sending time and distance for nodes, else enter n to generate them randomly" << endl;

    // Transmission power in dBm and noise power in dBm
    double Pt_dBm = 20.0;  // Transmit power in dBm (example value)
    double NoisePower_dBm = -101;  // Noise power in dBm (example value)

    char c;
    cin >> c;
    if (c == 'y') {
        for (int i = 0; i < numNodes; i++) {
            int dataSendingTime;
            double distance;
            cout << "Enter Data Sending Time and the distance b/w the Device and Receiver " << i << ": ";
            cin >> dataSendingTime >> distance;
            nodes.push_back(Node(i, dataSendingTime, distance, Pt_dBm, NoisePower_dBm));
        }
    } else {
        cout << "Automatically generated data sending times & distance b/w devices are respectively" << endl;

        for (int i = 0; i < numNodes; i++) {
            int dataSendingTime = (rand() % maxPacketLength) + 1;
            double distance = (rand() % 700) + 1.8;
            nodes.push_back(Node(i, dataSendingTime, distance, Pt_dBm, NoisePower_dBm));
            cout << "Device " << i << " : " << dataSendingTime << " , " << distance << endl;
        }
    }

    // Demonstration in every time slot
    for (int slot = 0; slot < numSlots; slot++) {
        cout << "\nTime " << slot + 1 << " :" << endl;

        set<int> oldChannel = oldchannel.channelNodes;

        // Remove the nodes from oldchannel which are done transmitting whole data
        if(slot>0)
        for (auto &node : nodes) {
            for (auto &id : oldChannel) {
                if (id == node.id && !node.donePartially()) {
                    if (oldchannel.channelNodes.find(id) != oldchannel.channelNodes.end())
                        oldchannel.channelNodes.erase(oldchannel.channelNodes.find(node.id));

                    if (node.snr >= snrThreshold) {
                        cout << "Device " << id << " has successfully transmitted its message in this time slot" << endl;
                        node.transmitted = true;
                        successfulTransmissions.push_back(id);
                    } else {
                        cout << "Device " << id << " failed due to low SNR (" << node.snr << ") and is blocked." << endl;
                        node.transmitted = true;
                        failedDueToSNR.push_back(id);
                        blockedNodes.push_back(id);
                    }
                }
            }
        }

        if (successfulTransmissions.size() + failedDueToSNR.size() == numNodes) {
            cout << "\nAll done transmitting\n";
            break;
        }

        // Nodes which are in oldchannel and left with some more data to transmit
        if(slot>0){
        cout << "Nodes of Prev transition still continuing: ";
        for (auto node : oldchannel.channelNodes) {
            cout << node << " ";
        }}
        cout << endl;

        // We can transmit data if oldchannel is not busy otherwise need to be backed off
        if(slot==0){
            for (auto &node : nodes) {
                if (!node.transmitted && node.backoff == 0) {
                    if (oldchannel.isIdle() || oldchannel.channelNodes.find(node.id) != oldchannel.channelNodes.end()) {
                        node.Backoff();
                    }
                }
            }
        }
        else{
            for (auto &node : nodes) {
                if (!node.transmitted && node.backoff == 0) {
                    if (oldchannel.isIdle() || oldchannel.channelNodes.find(node.id) != oldchannel.channelNodes.end()) {
                        node.transmit(channel);
                    } else {
                        node.Backoff();
                    }
                }
            }
        }

        // These are the nodes which are now interested in transmitting data
        cout << "Nodes of current transition: ";
        for (auto &node : channel.channelNodes) {
            cout << node << " ";
        }
        cout << endl;

        // If more than 1 node is interested then collision happens (collision detection)
        // all the nodes need to be backed off, else it can go successfully
        if (channel.channelNodes.size() > 1) {
            for (auto &node : nodes) {
                for (auto &id : channel.channelNodes) {
                    if (id == node.id) {
                        node.Collide();
                        node.Backoff();
                    }
                }
            }
            Totalcollisions++;
            channel.clear();
        }
        // Make the channel as oldchannel for the new iteration and clear the current channel
        oldchannel = channel;
        channel.clear();

        // Decrementing backoff value in every iteration
        for (auto &node : nodes) {
            node.decrementBackoff();
        }
    }

        // Report from the above demonstration
    cout << "Number of nodes which Successfully Transmitted entire message: " << successfulTransmissions.size() << endl;

    cout << "And the nodes are:  ";
    sort(successfulTransmissions.begin(), successfulTransmissions.end());
    for (auto &x : successfulTransmissions) {
        cout << x << " ";
    }
    if (successfulTransmissions.size() == 0) {
        cout << "None";
    }
    cout << endl;

    cout << "Total Number of Collisions: " << Totalcollisions << endl;
    
    cout << "Backoff Attempts: " << endl;
    int sumBackoffAttempts = 0;
    for (auto &node : nodes) {
        cout << "Node " << node.id << ": " << node.backoffAttempts << endl;
        sumBackoffAttempts += node.backoffAttempts;
    }
    cout << "Total Number of Backoff Attempts: " << sumBackoffAttempts << endl;

    cout << "Nodes blocked due to poor SNR: ";
    for (auto &x : blockedNodes) {
        cout << x << " ";
    }
    if (blockedNodes.size() == 0) {
        cout << "None";
    }
    cout << endl << endl;

    return 0;
}
