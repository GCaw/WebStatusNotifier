#include "InternetButton/InternetButton.h"
#include "application.h"
#include "HttpClient/HttpClient.h"
#include "math.h"

unsigned int nextTime = 0;    // Next time to contact the server
HttpClient http;
InternetButton b = InternetButton();
String hostDomains [] = {"domain1", "domain2", "domain3", "domain4"}; //type in domain names to check here eg. "google.com"
bool specialCheck [] = {0,0,0,0,0};

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void setup() {
    b.begin();
    b.ledOn(1,0,0,255); //blue light to show initialised
}

void loop() {
    
    if (nextTime > millis()) {
        
        if(b.buttonOn(3)) //button 3 can be triggered to start check, otherwise loops until predetermined time
        {
            nextTime = millis();
            delay(20);
        }
        
        int xValue = b.readX(); //poll accelerometer for x accleration sideways)
        if (abs(xValue) > 50) 
        {
            specialCheck [0] = true; //perform special check
            nextTime = millis();
            delay(20);
        }
        return;
    }
    
    for (int i = 1; i < 5; i++) //4 websites to check, 4 iterations
    {
        if ((specialCheck[0] && specialCheck[i]) || !specialCheck[0])
        {
            b.ledOn((2*i)+3,255,107,66); //if a site is being checked light goes orangey
            
            request.hostname = hostDomains[i-1];
            request.port = 80;
            request.path = "/upordown/index.html"; //where to store site that must get checked
        
            http.get(request, response, headers); //send http request
        
            if(response.body == "writing on website")   //check if the correct website is loaded
            {
                b.ledOn(3+(2*i),0,255,0); //green
                //Particle.publish(request.hostname, response.body);// fault checking
                specialCheck [i] = 0;
            }
            else
            {
                b.ledOn(3+(2*i),255,0,0); //red
                Particle.publish(request.hostname, response.body);// prints website and text when a fault
                b.playNote("G3",8); //plays a tone when there's an error. will probably disable
                specialCheck [i] = 1;
            }
            
            response.body = "no"; //I found that if a server was not found, the response.body was not updated, so kept previous value
            // need to work on better differentiation of problem
            // add if failure to try again sooner? other notification?.
        }
    }
    
    specialCheck [0] = 0;
    nextTime = millis() + 600000; //wait ten minutes till next check
}
