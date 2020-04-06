#include "Input.h"

std::vector<SR::IInputListener*> SR::IInputListener::keyBoardListeners, 
SR::IInputListener::mouseButtonListeners,
SR::IInputListener::mouseMoveListeners,
SR::IInputListener::wheelScrollListeners;
float SR::Input::mouseX = 0;
float SR::Input::lastX = 0;
float SR::Input::mouseY = 0;
float SR::Input::lastY = 0;

bool SR::Input::keyDown[int(SR::KeyCode::KeyNum)] = { false };
bool SR::Input::keyUp[int(SR::KeyCode::KeyNum)] = { false };

