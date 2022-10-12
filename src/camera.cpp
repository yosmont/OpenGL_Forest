#include <camera.hpp>

#pragma warning(push, 0)
#include <glm/gtc/matrix_transform.hpp>
#pragma warning(pop, 0)

BEGIN_VISUALIZER_NAMESPACE

Camera::Camera(uint32_t windowWidth,
			   uint32_t windowHeight,
			   const glm::vec3& position,
			   float horizontalAngle,
			   float verticalAngle,
			   float speed,
			   float mouseSpeed,
			   float FOV,
		       float Near,
			   float Far)
    : m_WindowWidth(windowWidth)
	, m_WindowHeight(windowHeight)
	, m_Position(position)
	, m_HorizontalAngle(horizontalAngle)
	, m_VerticalAngle(verticalAngle)
	, m_MovementSpeed(speed)
	, m_MouseMovementSpeed(mouseSpeed)
	, m_FOV(FOV)
	, m_Near(Near)
,	  m_Far(Far)
{
    m_Right = glm::vec3(glm::cos(m_HorizontalAngle - glm::half_pi<float>()), 0.f, glm::sin(m_HorizontalAngle - glm::half_pi<float>()));

	m_Direction.x = glm::cos(m_VerticalAngle) * glm::cos(m_HorizontalAngle);
	m_Direction.y = glm::sin(m_VerticalAngle);
	m_Direction.z = glm::cos(m_VerticalAngle) * glm::sin(m_HorizontalAngle);
	
	m_Up = glm::cross(m_Right, m_Direction);

    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	ComputeProjection(m_WindowWidth, m_WindowHeight);
}

void Camera::VerticalMovement(int movement)
{
	m_VerticalAngle = glm::clamp(m_VerticalAngle + m_MouseMovementSpeed * movement, glm::half_pi<float>(), 3.0f * glm::pi<float>() / 2.0f);

    m_Direction.x = glm::cos(m_VerticalAngle) * glm::cos(m_HorizontalAngle);
    m_Direction.y = glm::sin(m_VerticalAngle);
    m_Direction.z = glm::cos(m_VerticalAngle) * glm::sin(m_HorizontalAngle);

	m_Up = glm::cross(m_Right, m_Direction);

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::HorizontalMovement(int movement)
{
	m_HorizontalAngle = glm::mod(m_HorizontalAngle + m_MouseMovementSpeed * movement, glm::two_pi<float>());

	m_Direction.x = glm::cos(m_VerticalAngle) * glm::cos(m_HorizontalAngle);
	m_Direction.z = glm::cos(m_VerticalAngle) * glm::sin(m_HorizontalAngle);

	m_Right.x = glm::cos(m_HorizontalAngle - glm::half_pi<float>());
	m_Right.z = glm::sin(m_HorizontalAngle - glm::half_pi<float>());

	m_Up = glm::cross(m_Right, m_Direction);

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::MoveForward(float dt)
{
	m_Position += m_Direction * m_MovementSpeed * dt;

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::MoveBackward(float dt)
{
	m_Position -= m_Direction * m_MovementSpeed * dt;

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::MoveLeft(float dt)
{
	m_Position -= m_Right * m_MovementSpeed * dt;

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::MoveRight(float dt)
{
	m_Position += m_Right * m_MovementSpeed * dt;

	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::ComputeProjection(uint32_t windowWidth, uint32_t windowHeight)
{
	m_WindowWidth = windowWidth;
	m_WindowHeight = windowHeight;
	
	m_AspectRatio = static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);

	m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight), m_Near, m_Far);

	m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
}

void Camera::SetFov(float FOV)
{
	m_FOV = glm::clamp(FOV, s_MinFov, s_MaxFov);

	ComputeProjection(m_WindowWidth, m_WindowHeight);
}

END_VISUALIZER_NAMESPACE
