#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>

#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include "Node.h"

using namespace std;

//A vector that keeps track of all the information about each node in the input file
vector<Node*> nodes;

struct passCommand{
	void enterCommand(string command, int source, int destination){
			struct sockaddr_in myAddr, remoteAddr;
	socklen_t remoteAddrLen = sizeof(remoteAddr);
	
	struct hostent *hostInfo;
	struct sockaddr_in servAddr;
	char msg[1024];
	
	//Create the socket for the Node
    int sd;
    if((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Cannot create socket");
        exit(1);
    }

    int enable = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    perror("setsockopt(SO_REUSEADDR) failed");

    
    memset((char *)&myAddr, 0, sizeof(myAddr));
	myAddr.sin_family = AF_INET;
	myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myAddr.sin_port = htons(0);
    
    if (bind(sd, (struct sockaddr*)&myAddr, sizeof(myAddr)) < 0) {
		perror("binding failed");
		exit(1);
	} 
	
	memset((char*)&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(nodes.at(source-1)->controlPort);

	//get the IP address of servhost
	struct hostent *tempStruct;
	if ((tempStruct = gethostbyname(nodes.at(source-1)->hostName.c_str())) == NULL) {
		fprintf(stderr, "Error while getting host name\n");
		exit(1);
	}
	
	struct in_addr **ipAddress;
    ipAddress = (struct in_addr **) tempStruct->h_addr_list;
	
	servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*ipAddress[0]));

	//Tell the source node to generate and send a packet to the destination node
	if(command == "generate-packet") {
	    string temp = "generate-packet " + to_string(destination) + " ";	
	    strcpy(msg, temp.c_str());
	
	    if(sendto(sd, msg, strlen(msg), 0, (struct sockaddr*)&servAddr, remoteAddrLen) == -1) {
		    perror("message sending failed");
	    }
	}
	
	//Tell the source and destination nodes to create or remove a link between them
	else {
		struct sockaddr_in servAddr2;
	
		memset((char*)&servAddr2, 0, sizeof(servAddr2));
		servAddr2.sin_family = AF_INET;
		servAddr2.sin_port = htons(nodes.at(source-1)->controlPort);
		
		struct hostent *tempStruct2;
		if ((tempStruct2 = gethostbyname(nodes.at(source-1)->hostName.c_str())) == NULL) {
			fprintf(stderr, "Error while getting host name\n");
			exit(1);
		}
	
		struct in_addr **ipAddress2;
		ipAddress2 = (struct in_addr **) tempStruct2->h_addr_list;
	
		servAddr2.sin_addr.s_addr = inet_addr(inet_ntoa(*ipAddress2[0]));
	
		string temp = command + " " + to_string(destination) + " ";	
		strcpy(msg, temp.c_str());
	
		if(sendto(sd, msg, strlen(msg), 0, (struct sockaddr*)&servAddr2, remoteAddrLen) == -1) {
			perror("message sending failed");
		}
		
		// -------------------------------------------------------
		
		struct sockaddr_in servAddr3;
	
		memset((char*)&servAddr3, 0, sizeof(servAddr3));
		servAddr3.sin_family = AF_INET;
		servAddr3.sin_port = htons(nodes.at(destination-1)->controlPort);
		
		struct hostent *tempStruct3;
		if ((tempStruct3 = gethostbyname(nodes.at(destination-1)->hostName.c_str())) == NULL) {
			fprintf(stderr, "Error while getting host name\n");
			exit(1);
		}
	
		struct in_addr **ipAddress3;
		ipAddress3 = (struct in_addr **) tempStruct3->h_addr_list;
	
		servAddr3.sin_addr.s_addr = inet_addr(inet_ntoa(*ipAddress3[0]));
	
		temp = command + " " + to_string(source) + " ";	
		strcpy(msg, temp.c_str());
	
		if(sendto(sd, msg, strlen(msg), 0, (struct sockaddr*)&servAddr3, remoteAddrLen) == -1) {
			perror("message sending failed");
		}
	}

	close(sd);

	}
};

void passcommandDummy(string command, int source, int destination){
	passCommand pc;
	return pc.enterCommand(command,source,destination);
}

void
usage(const std::string& program_name)
{
	std::cout << program_name << " <command> <sourceID> <destinationID> " << std::endl;
}


int main(int argc, char *argv[]) {
		if (argc < 3){
  	usage(std::string(argv[0]));
  	exit(0);
  }
	string fileName = "input.txt";

	std::ifstream inputFile(fileName);
	
	string command(argv[1]);
	int source = atoi(argv[2]);
	int destination = atoi(argv[3]);
	if (inputFile.is_open()) {
		string newString;
		int nodeID, controlPort, dataPort;
		string hostName;
		
		//read input file to get information about all the nodes in the network, and store it in the list
		while (getline(inputFile, newString)) {
			std::istringstream ss1;
			ss1.str(newString);
			ss1 >> nodeID >> hostName >> controlPort >> dataPort;
			
			Node* tempNode = new Node(nodeID, hostName, controlPort, dataPort);
			
			string delimiter = "\t";
			size_t pos = 0;
			string token;
			int count = 0;
			
			while ((pos = newString.find(delimiter)) != string::npos) {
				token = newString.substr(0, pos);
				newString.erase(0, pos + delimiter.length());
			
				if(count < 4) {
					count++;
				}
				
				else {
					std::cout << "token 1 = " << token << std::endl;
					const char *c_str = token.c_str();
					tempNode->nebularAdd(atoi(c_str));
				}
			}
			
					std::cout << "token 2 = " << token << std::endl;
					const char *c_str = token.c_str();
					tempNode->nebularAdd(atoi(c_str));
			
			nodes.push_back(tempNode);
		}
		
		inputFile.close();
			
	} else cout << "\nUnable to open file." << endl;
	
	if(source != destination && source <= nodes.size() && destination <= nodes.size() && source > 0 && destination > 0) {
		passcommandDummy(command, source, destination);
	}
	else {
		cerr << "Node values must not be the same and must be valid current node IDs." << endl;
	}
}