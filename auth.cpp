#include <iostream>
#include <string>
#include "auth.h"

using namespace std; 



void authentication()
{
string username, password; 

username = "admin";
password = "adminpass";

cout << "Username: " << endl;
cin >> username;
cout << "Password" << endl;
cin >> password;

if (username == "admin" && password == "adminpass")
     {
         cout << "Success!! System ready and booted!" << endl;
     }
else 
    {
     cout << "Incorrect username or password" << endl;
    }
   
}