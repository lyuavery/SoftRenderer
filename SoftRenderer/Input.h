#pragma once
#include <vector>
#include "Math/Vector.h"
#include "Sealed.h"
#include "Log.h"
namespace SR
{
	enum class KeyCode
	{
		None = 0,
		Return,
		Escape,
		Space,
		Alpha0,
		Alpha1,
		Alpha2,
		Alpha3,
		Alpha4,
		Alpha5,
		Alpha6,
		Alpha7,
		Alpha8,
		Alpha9,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Pad0,
		Pad1,
		Pad2,
		Pad3,
		Pad4,
		Pad5,
		Pad6,
		Pad7,
		Pad8,
		Pad9,
		UpArrow,
		DownArrow,
		RightArrow,
		LeftArrow,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		LeftCtrl,
		RightCtrl,
		LeftMouse,
		MiddleMouse,
		RightMouse,
		Tab,
		KeyNum,
	};
	// 
	class IInputListener
	{
	protected:
		IInputListener() {}
		virtual ~IInputListener() {}
	public:
		static std::vector<IInputListener*> keyBoardListeners;
		static std::vector<IInputListener*> mouseButtonListeners;
		static std::vector<IInputListener*> mouseMoveListeners;
		static std::vector<IInputListener*> wheelScrollListeners;
		virtual void HandleKeyBoardMsg() = 0;
		virtual void HandleMouseButtonMsg(KeyCode code, bool bClicked, float posX, float posY) = 0;
		virtual void HandleMouseMoveMsg(float posX, float posY) = 0;
		virtual void HandleWheelScrollMsg(float offset) = 0;
	};

	template<typename T>
	class InputListener : public IInputListener
	{
	public:
		typedef void (T::*KeyBoardMsgHandler)();
		typedef void (T::*MouseButtonMsgHandler)(KeyCode code, bool bClicked, float posX, float posY);
		typedef void (T::*MouseMoveMsgHandler)(float posX, float posY);
		typedef void (T::*WheelScrollMsgHandler)(float offset);
	private:
		// ((&b.a)->*(b.F))();
		T* obj = nullptr;
		KeyBoardMsgHandler onKeyBoardMsg = nullptr;
		MouseButtonMsgHandler onMouseButtonMsg = nullptr;
		MouseMoveMsgHandler onMouseMoveMsg = nullptr;
		WheelScrollMsgHandler onWheelScrollMsg = nullptr;

		InputListener<T>() = delete;
		InputListener<T>(const InputListener<T>&) = delete;
		InputListener<T>(InputListener<T>&&) = delete;
		InputListener<T>& operator=(InputListener<T>&&) = delete;
		InputListener<T>& operator=(const InputListener<T>&) = delete;
	protected:
		virtual void HandleKeyBoardMsg() override
		{
			if (!obj || !onKeyBoardMsg) return;
			(obj->*onKeyBoardMsg)();
		}
		virtual void HandleMouseButtonMsg(KeyCode code, bool bClicked, float posX, float posY) override
		{
			if (!obj || !onMouseButtonMsg) return;
			(obj->*onMouseButtonMsg)(code, bClicked, posX, posY);
		}
		virtual void HandleMouseMoveMsg(float posX, float posY) override
		{
			if (!obj || !onMouseMoveMsg) return;
			(obj->*onMouseMoveMsg)(posX, posY);
		}
		virtual void HandleWheelScrollMsg(float offset) override
		{
			if (!obj || !onWheelScrollMsg) return;
			(obj->*onWheelScrollMsg)(offset);
		}

	public:
		InputListener(T* o, KeyBoardMsgHandler kb, MouseButtonMsgHandler mb, MouseMoveMsgHandler mm, WheelScrollMsgHandler ws);
		~InputListener();
	};


	// 输入事件应该独立于帧率，所以不应该用Update的方式通知Listeners
	class Input : private virtual ISealed<Input>
	{
		friend class Window;
	private:		
		// 什么时候复位信号
		static bool keyDown[int(KeyCode::KeyNum)];
		static bool keyUp[int(KeyCode::KeyNum)];
		static float mouseX, lastX;
		static float mouseY, lastY;

		static void SetKeyDown(KeyCode code) { 
			if (code >= KeyCode::KeyNum) return;
			keyDown[int(code)] = true; keyUp[int(code)] = false;
		}
		static void SetKeyUp(KeyCode code) {
			if (code >= KeyCode::KeyNum) return;
			keyDown[int(code)] = false; keyUp[int(code)] = true;
		}
		static void SetCursorPos(float x, float y)
		{
			lastX = mouseX; lastY = mouseY;
			mouseX = x; mouseY = y;
		}
		static void ResetKey()
		{
			memset(&keyUp, 0, int(KeyCode::KeyNum));
		}
	public:
		static inline bool GetKeyDown(KeyCode code) { 
			return code >= KeyCode::KeyNum ? false : keyDown[int(code)];
		}
		static inline bool GetKeyUp(KeyCode code) {
			return code >= KeyCode::KeyNum ? false : keyUp[int(code)];
		}
		static inline Vec2 GetCursorPos() { 
			return Vec2(mouseX, mouseY); 
		}
		static inline Vec2 GetLastCursorPos() { 
			return Vec2(lastX, lastY); 
		}
		static void NotifyKeyBoardListeners() {
			for (auto listener : IInputListener::keyBoardListeners) { listener->HandleKeyBoardMsg(); }
		}

		static void NotifyMouseButtonListeners(KeyCode code, bool bClicked, float posX, float posY) {
			for (auto listener : IInputListener::mouseButtonListeners) { listener->HandleMouseButtonMsg(code, bClicked, posX, posY); }
		}

		static void NotifyMouseMoveListeners(float posX, float posY) {
			for (auto listener : IInputListener::mouseMoveListeners) { listener->HandleMouseMoveMsg(posX, posY); }
		}

		static void NotifyWheelScrollListeners(float offset) {
			for (auto listener : IInputListener::wheelScrollListeners) { listener->HandleWheelScrollMsg(offset); }
		}

		static void Update(float delta)
		{
			NotifyKeyBoardListeners();
		}
	};

	template<typename T>
	InputListener<T>::InputListener(T* o, KeyBoardMsgHandler kb, MouseButtonMsgHandler mb, MouseMoveMsgHandler mm, WheelScrollMsgHandler ws) :
		obj(o), onKeyBoardMsg(kb), onMouseButtonMsg(mb), onMouseMoveMsg(mm), onWheelScrollMsg(ws)
	{
		if (!o) return;
		if (onKeyBoardMsg) keyBoardListeners.push_back(this);
		if (onMouseButtonMsg) mouseButtonListeners.push_back(this);
		if (onMouseMoveMsg) mouseMoveListeners.push_back(this);
		if (onWheelScrollMsg) wheelScrollListeners.push_back(this);
	}

	template<typename T>
	InputListener<T>::~InputListener()
	{
		std::vector<SR::IInputListener*>* listeners[] = { &keyBoardListeners, &mouseButtonListeners, &mouseMoveListeners, &wheelScrollListeners };
		for (auto* ptr : listeners)
		{
			auto it = find((*ptr).begin(), (*ptr).end(), this);
			if (it != (*ptr).end()) (*ptr).erase(it);
		}
	}

}


