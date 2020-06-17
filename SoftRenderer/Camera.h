#pragma once
#include <memory>
#include "Math/Vector.h"
#include "Math/Matrix4x4.h"
#include "Viewport.h"
#include "Input.h"
namespace SR
{
	class Camera
	{
		enum class Mode
		{
			None = 0,
			Pan = 1 << 0,
			Orbit = 1 << 1,
			Rotate = 1 << 2,
		};

		int mode = 0;
		Vec3 worldUp = Vec3(0.0f, 1.0f, 0.0f); // 世界空间上向量
		Vec3 center = Vec3(0);
		std::unique_ptr<InputListener<Camera>> inputListener;
		void UpdateCameraBasis();
		void OnKeyBoardMsg();
		void OnMouseBottonMsg(SR::KeyCode code, bool bClicked, float posX, float posY);
		void OnMouseMove(float posX, float posY);
		void OnWheelScrollMsg(float offset);
	public:
		// World Space 
		Vec3 position = Vec3(0, 0, 0);// 相机位置
		Vec3 front = Vec3(0, 0, 1);// 相机指向(观察目标)的方向，与View Space的前向量相反
		Vec3 up = Vec3(0, 1, 0);// 上向量（+Y)
		Vec3 right = Vec3(1, 0, 0);// 右向量 (+X)
		float fov = 60; //Field of View
		
		float nearClip = 0.1f;
		float farClip = 100;
		float orthoSize = 1;
		bool bOrtho = false;
		Viewport viewport = Viewport(0, 0, 1, 1, false);

		static std::unique_ptr<Camera> mainCamera;

		Camera(const Vec3& position, const Vec3& target);
		~Camera() {}
		Mat4 ViewMatrix();
		Mat4 ProjectionMatrix();
		Mat4 ViewportTransform();
		float GetAspect() const;
		void LookAt(const Vec3& target);
		void RegisterInputListener();
		void UnregisterInputListener();
		// Camera options
		float movementSpeed = 5;
		float angularSpeed = 0.5f;
		float zoomSpeed = 10;
		float mouseDragSensitivity = .03f;
		float mouseRotateSensitivity = .08f;
	
		void Dolly(const Vec3& deltaTranslation);
		void Pan(float deltaX, float deltaY);
		void Orbit(float deltaPhi, float deltaTheta, bool constrainPitch = true);
		void Rotate(float deltaYaw, float deltaPitch, bool constrainPitch = true);	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
		void Zoom(float offset);
	};
}
