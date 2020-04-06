#include "Camera.h"
#include "Input.h"
#include "Time.h"
#include "Log.h"
std::unique_ptr<SR::Camera> SR::Camera::mainCamera(nullptr);

//SR::Camera::Camera():
	//position(Vec3(0,0,0)),
	//front(Vec3(0,0,1)),
	//up(Vec3(0, 1, 0)),
	//right(Vec3(1,0,0)),
	//worldUp(Vec3(0.0f, 1.0f, 0.0f)),
	//yaw(270.0f),
	//pitch(.0f),
	//fov(45.0f),
	//nearClip(0.1f),
	//farClip(100.f),
	//orthoSize(1),
	//bOrtho(false),
	//viewport(0, 0, 1, 1, false)
	//movementSpeed(5.0f),
	//zoomSpeed(25.0f),
	//angularSpeed(0.5f),
	//mouseDragSensitivity(0.03f),
	//mouseRotateSensitivity(0.06f),
	//bProcessMouseDragEnabled(false),
	//bProcessMouseRotateEnabled(false)
//{
//}

SR::Camera::Camera(const Vec3& position, const Vec3& target)
{
	this->position = position;
	LookAt(target);
}
//
//SR::Camera::Camera(const Vec3& position, const Vec3& up, float yaw, float pitch)
//{
//	this->position = position;
//	worldUp = up;
//	this->yaw = yaw;
//	this->pitch = pitch;
//	UpdateCameraBasis();
//}

void SR::Camera::LookAt(const Vec3& center)
{
	// 右手系
	this->center = center;
	front = -(center - position).Normalized();
	if (front == Vec3::zero) front = -Vec3::front;
	right = sbm::Cross(worldUp, front).Normalized();
	up = sbm::Cross(front, right).Normalized();
	//pitch = sbm::degrees(sbm::asin(-front.y)); // 这里不反过来求出姿态角的话，在update vector的时候会瞬间转向
	//yaw = (sbm::Dot(-front, Vec3(0, 0, -1)) > 0 ? 1 : -1) *
	//	sbm::degrees(
	//		sbm::acos(-front.x / (sbm::cos(sbm::radians(pitch))))
	//	); //acos0时无法判断时+90°还是-90°，这时候我用相机右轴指向世界坐标的方向性来判断
}

Mat4 SR::Camera::ViewMatrix()
{
	return Mat4::LookAt(position, position - front, worldUp);
}

float SR::Camera::GetAspect() const
{
	int w = viewport.width, h = viewport.height;
	if (w == 0 || h == 0) return 1;
	return sbm::abs(w * (1.f / h));
}

Mat4 SR::Camera::ProjectionMatrix()
{
	Mat4 proj = Mat4::Identity;
	float aspect = GetAspect();
	float fovy = sbm::radians(fov);
	if (bOrtho)
	{
		proj.M(0, 0) = 1.f / (aspect * orthoSize);
		proj.M(1, 1) = 1.f / orthoSize;
		proj.M(2, 2) = -2.f / (farClip - nearClip);
		proj.M(2, 3) = -(farClip + nearClip) / (farClip - nearClip);
	}
	else
	{
		proj.M(0, 0) = sbm::cot(fovy/2.f) / aspect;
		proj.M(1, 1) = sbm::cot(fovy / 2.f);
		proj.M(2, 2) = -(farClip + nearClip) / (farClip - nearClip) ;
		proj.M(2, 3) = -2.f * farClip * nearClip / (farClip - nearClip);
		proj.M(3, 2) = -1;
		proj.M(3, 3) = 0;
	}
	return proj;
}

Mat4 SR::Camera::ViewportTransform()
{
	return viewport.GetViewportMatrix();
}

void SR::Camera::RegisterInputListener()
{
	if (inputListener) return;
	//SR::InputListener<SR::Camera> x = SR::InputListener<SR::Camera>();
	SR::InputListener<SR::Camera>* ptr = new SR::InputListener<SR::Camera>(this, 
		&SR::Camera::OnKeyBoardMsg, &SR::Camera::OnMouseBottonMsg, &SR::Camera::OnMouseMove, &SR::Camera::OnWheelScrollMsg );
	//inputListener = std::unique_ptr<SR::InputListener<SR::Camera>>(ptr);
}

void SR::Camera::UnregisterInputListener()
{
	if (!inputListener) return;
	inputListener.reset();
}

void SR::Camera::OnKeyBoardMsg()
{
	float distanceDelta = movementSpeed * SR::Time::DeltaTime();
	if (SR::Input::GetKeyDown(SR::KeyCode::UpArrow) || SR::Input::GetKeyDown(SR::KeyCode::W))
	{
		Dolly(-front * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::DownArrow) || SR::Input::GetKeyDown(SR::KeyCode::S))
	{
		Dolly(front * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::LeftArrow) || SR::Input::GetKeyDown(SR::KeyCode::A))
	{
		Dolly(-right * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::RightArrow) || SR::Input::GetKeyDown(SR::KeyCode::D))
	{
		Dolly(right * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::Q))
	{
		Dolly(-worldUp * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::E))
	{
		Dolly(worldUp * distanceDelta);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::Tab))
	{
		center = Vec3::zero;
		position = Vec3(0, 0, 3);
		LookAt(center);
	}
	if (SR::Input::GetKeyDown(SR::KeyCode::LeftCtrl) || SR::Input::GetKeyDown(SR::KeyCode::RightCtrl))
	{
		mode |= int(Mode::Rotate);
	}
	else if(SR::Input::GetKeyUp(SR::KeyCode::LeftCtrl) || SR::Input::GetKeyUp(SR::KeyCode::RightCtrl))
	{
		mode &= ~int(Mode::Rotate);

	}
}

void SR::Camera::OnMouseBottonMsg(SR::KeyCode code, bool bClicked, float posX, float posY)
{
	switch (code)
	{
	case SR::KeyCode::LeftMouse:
	{
		if (bClicked) {
			if (!(mode & int(Mode::Pan)))
			{
				mode |= int(Mode::Orbit);
			}
		}
		else {
			mode &= ~int(Mode::Orbit);
		}
		break;
	}
	case SR::KeyCode::RightMouse:
	{
		if (bClicked) {
			if (!(mode & int(Mode::Orbit)))
			{
				mode |= int(Mode::Pan);
			}
		}
		else {
			mode &= ~int(Mode::Pan);
		}

		break;
	}
	default:
		break;
	}
}

void SR::Camera::OnMouseMove(float posX, float posY)
{
	auto&& pos = SR::Input::GetLastCursorPos();
	float xOffset = posX - pos.x, yOffset = posY - pos.y;
	if (mode & int(Mode::Orbit))
	{
		if (mode & int(Mode::Rotate))
		{
			//XLogInfo("Rotating! X: %lf, Y: %lf", xOffset * mouseRotateSensitivity , -yOffset * mouseRotateSensitivity);
			Rotate(-xOffset * mouseRotateSensitivity, -yOffset * mouseRotateSensitivity);
		}
		else
		{
			//XLogInfo("Orbiting! X: %lf, Y: %lf", xOffset * angularSpeed, yOffset * angularSpeed);
			// +y offset -theta, +x offset - phi
			Orbit(-xOffset * angularSpeed, -yOffset * angularSpeed);
		}
	}
	else if (mode & int(Mode::Pan))
	{
		//XLogInfo("Panning! X: %lf, Y: %lf", xOffset * mouseDragSensitivity, yOffset * mouseDragSensitivity);
		Pan(xOffset * mouseDragSensitivity, yOffset * mouseDragSensitivity);
	}
}

void SR::Camera::OnWheelScrollMsg(float offset)
{
	Zoom(offset * zoomSpeed);
}

void SR::Camera::Dolly(const Vec3& deltaTranslation)
{
	center += deltaTranslation;
	position += deltaTranslation;
}

void SR::Camera::Pan(float deltaX, float deltaY)
{
	// 这里没有用deltaTime是因为这个移动不是像上面一样走循环更新的，而是矢量回调更新的
	Dolly(-right * deltaX + up * deltaY);
}

void SR::Camera::Orbit(float deltaPhi, float deltaTheta, bool constrain)
{
	// 右手系，-front==Vector::front时phi为90度，-front==Vector::right时phi为0度
	Vec3 relativePos = position - center;
	float radius = relativePos.Magnitude();
	relativePos.Normalize();
	float theta = sbm::acos(sbm::clamp(relativePos.y,-1.f,1.f));
	float phi = (relativePos.z > 0 ? -1 : 1) * sbm::acos(sbm::clamp(relativePos.x / sbm::sin(theta),-1.f,1.f));
	phi += sbm::radians(deltaPhi); // in degrees
	theta += sbm::radians(deltaTheta); // in degrees
	if (constrain)
	{
		theta = sbm::clamp<float>(theta, 0.1f, sbm::Math::PI-0.1f);
	}
	Vec3 newLocalPos = Vec3(
		radius * sbm::sin(theta) * sbm::cos(phi),
		radius * sbm::cos(theta),
		- radius * sbm::sin(theta) * sbm::sin(phi)
	);
	position = newLocalPos + center;
	front = position - center;

	UpdateCameraBasis();
}

void SR::Camera::Rotate(float deltaYaw, float deltaPitch, bool constrain)
{	
	Mat4 rotation = Mat4::Rotate(Vec3(deltaPitch, deltaYaw, 0));
	auto distance = (center - position).Magnitude();
	front = (rotation * front.Expanded<4>(0)).Truncated<3>();
	if (constrain)
	{
		front.Normalize();
		if (sbm::abs(front.y) > 0.99f)
		{
			front.y = 0.99f * (front.y > 0 ? 1 : -1);
			front.Normalize();
		}
	}
	center = position + distance * -front;
	UpdateCameraBasis();
}

void SR::Camera::Zoom(float yoffset)
{
	if (fov >= 1.0f && fov <= 75.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 75.0f)
		fov = 75.0f;
}

void SR::Camera::UpdateCameraBasis()
{
	front.Normalize();	
	right = sbm::Cross(worldUp, front).Normalized();
	up = sbm::Cross(front, right).Normalized();
}
