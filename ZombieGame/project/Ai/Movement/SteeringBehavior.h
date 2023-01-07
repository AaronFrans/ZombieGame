#include <type_traits>
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"


class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) = 0;

	//Seek Functions
	void SetTarget(const Elite::Vector2& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(this);
	}

protected:
	Elite::Vector2 m_Target;

};


///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;
};


///////////////////////////////////////
//Wander
//****
class ExploreWorld : public ISteeringBehavior
{
public:
	ExploreWorld() = default;
	virtual ~ExploreWorld() = default;

	//Wander Behaviour
	SteeringPlugin_Output CalculateSteering(float deltaT, AgentInfo pAgent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; };
	void SetWanderRadius(float radius) { m_Radius = radius; };
	void SetMaxAngleChange(float angleChange) { m_MaxAngleChange = angleChange; };


protected:

	float m_OffsetDistance = 6.f;
	float m_Radius = 15.f;
	float m_MaxAngleChange = Elite::ToRadians(90);
	float m_WanderAngle = 0.f;
};