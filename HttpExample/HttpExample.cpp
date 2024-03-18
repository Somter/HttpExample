#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
#include <sstream>  
#include <ctime>

using namespace std;    

string unixTimeToStr(time_t unixTime) { //time_t - это тип данных, который используется для работы с временем.
    char buff[90];
    struct tm timeinfo;
    localtime_s(&timeinfo, &unixTime);
    strftime(buff, sizeof(buff), "%H:%M:%S", &timeinfo);   
    return string(buff);    
}


int main()
{
    setlocale(0, "ru");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  

    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }  

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться
   
    char hostname[255] = "api.openweathermap.org";
    
    addrinfo* result = NULL;    
    
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname, "http", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }     

    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуем присоединиться к полученному адресу
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //2. создание клиентского сокета
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

       //3. Соединяемся с сервером
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //4. HTTP Request

    string uri = "/data/2.5/weather?q=Odessa&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON";

    string request = "GET " + uri + " HTTP/1.1\n"; 
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";   
    request += "Connection: close\n";   
    request += "\n";

    //отправка сообщения
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0) {
            response += string(resBuf).substr(0, respLength);           
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }

    } while (respLength == BUFFERSIZE);

   istringstream iss(response);        
   string token;       

    string date;            
    while (iss >> token) {     
        if (token == "Date:") {     
            getline(iss, date);         
            break;
        }
    }

 
    string country;     
    size_t countryWords = response.find("\"country\":\"");    
	countryWords += 11;
	size_t lastletter = response.find('"', countryWords);
	country = response.substr(countryWords, lastletter - countryWords);  //substr - используется для извлечения подстроки из строки.
    
    string key = "name";
    string city;
    size_t keyPos = response.find("\"" + key + "\":\"");
	keyPos += key.length() + 4;
	size_t endPos1 = response.find('"', keyPos);      
	city = response.substr(keyPos, endPos1 - keyPos);
  
    string coords;  
    size_t coordPos = response.find("\"coord\":{");   
	coordPos += 9;
	coords = response.substr(coordPos);
	size_t endPos2 = coords.find("}");
	coords = coords.substr(0, endPos2); 
    
    double temp;
    size_t tempPos = response.find("\"temp\":");       
	tempPos += 7;
	size_t endPos3 = response.find(',', tempPos);
	string tempString = response.substr(tempPos, endPos3 - tempPos);
	temp = stof(tempString);
           
    int sunrise;
    size_t sunrisePos = response.find("\"sunrise\":"); 
    sunrisePos += 10;
	size_t endPos4 = response.find(',', sunrisePos);      
	string sunriseString = response.substr(sunrisePos, endPos4 - sunrisePos);   
	sunrise = stoi(sunriseString);
       
    int sunset; 
    size_t sunsetPos = response.find("\"sunset\":");    
	sunsetPos += 9;
	size_t endPos5 = response.find(',', sunsetPos);
	string sunsetString = response.substr(sunsetPos, endPos5 - sunsetPos);
	sunset = stol(sunsetString);
             
    SetConsoleTextAttribute(hConsole, 14);   
    cout << "Date:"; 
    SetConsoleTextAttribute(hConsole, 10);
    cout << date << endl;       

    SetConsoleTextAttribute(hConsole, 14);  
    cout << "Country: "; 
    SetConsoleTextAttribute(hConsole, 10);
    cout << country << endl;

    SetConsoleTextAttribute(hConsole, 14);  
    cout << "City: ";   
    SetConsoleTextAttribute(hConsole, 10);  
    cout << city << endl;   

    SetConsoleTextAttribute(hConsole, 14);  
    cout << "Coordinates: ";    
    SetConsoleTextAttribute(hConsole, 10);  
    cout << coords << endl; 

    SetConsoleTextAttribute(hConsole, 14);   
    cout << "Temperature: ";    
    SetConsoleTextAttribute(hConsole, 10);  
    cout << temp << endl;   

    SetConsoleTextAttribute(hConsole, 14);  
    cout << "Sunrise: ";    
    SetConsoleTextAttribute(hConsole, 10);      
    cout << unixTimeToStr(sunrise) << endl;

    SetConsoleTextAttribute(hConsole, 14);      
    cout << "Sunset: ";
    SetConsoleTextAttribute(hConsole, 10);      
    cout << unixTimeToStr(sunset) << endl;  

    SetConsoleTextAttribute(hConsole, 7);

    //отключает отправку и получение сообщений сокетом
    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;   
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();
}