#include "SMObject.h"
#include <SMStructs.h>
#include <conio.h>
#include <tlhelp32.h>
#include <iostream>

#include <Windows.h>
#include <xinput.h>
#include <math.h>
#include <conio.h>
#include "XinputWrapper.h"
#include "XBoxController.h"

#define PI 3.14159265

using namespace std;
using namespace System;
using namespace System::Threading;

int main()
{
	//DWORD user_ID = 0;

	XInputWrapper xinput;
	GamePad::XBoxController controller(&xinput, 0);

	//XINPUT_STATE State;

	if (controller.IsConnected() == 1) {
		std::cout << "Xboxcontroller is already connected." << std::endl;
	}
	else {
		std::cout << "Xboxcontroller is not connected yet............." << std::endl;
	}


	while (1) {
		controller.IsConnected();

		if (controller.PressedA() == 1) {
			std::cout << "You are pressing button A..............." << std::endl;
		}
		else {
			//std::cout << "You are not pressing button A." << std::endl;
		}

		//Right thumb joystick represent steering
		float R_X = controller.RightThumbLocation().GetX();
		float deadzone = 5000.0;
		if (R_X == -32768)
			R_X = -32767;

		float angle_scaled;
		if (R_X > 5000.0) {
			angle_scaled = (R_X - deadzone) * 32767.0 / (32767.0 - deadzone);
		}
		else if (R_X < -5000.0) {
			angle_scaled = (R_X + deadzone) * (-32767.0) / (-32767.0 + deadzone);
		}
		else {
			angle_scaled = 0;
		}
		float steerAngle = 40 * angle_scaled / 32767;
		std::cout << steerAngle << "   ";

		//Left thumb joystick represent speed
		float L_Y = controller.LeftThumbLocation().GetY();
		if (L_Y == -32768)
			L_Y = -32767;

		float YLength_scaled;
		if (L_Y > 5000.0) {
			YLength_scaled = (L_Y - deadzone) * 32767.0 / (32767.0 - deadzone);
		}
		else if (L_Y < -5000.0) {
			YLength_scaled = (L_Y + deadzone) * (-32767.0) / (-32767.0 + deadzone);
		}
		else {
			YLength_scaled = 0;
		}

		float speed = 1 * YLength_scaled / 32767;
		std::cout << speed << std::endl;

		Sleep(10);
	}
	_getch();
	return 0;
}