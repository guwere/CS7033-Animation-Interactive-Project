#include "Curve.hpp"
#include "GameWorld.hpp"
#include "GameObject.hpp"
#include "ShaderProgram.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::vector;
CubicCurve::CubicCurve(CurveType type)
	:m_Type(type),m_Samples(0)
{
}
CubicCurve::CubicCurve(CurveType type, glm::mat4x3 controlPoints, unsigned int samples)
	:m_Type(type),m_ControlPoints(controlPoints),m_Samples(samples)
{
	//createLine();
}

CubicCurve::CubicCurve()
	: m_Type(CurveType::Bezier), m_Samples(0)

{

}

CubicCurve::~CubicCurve()
{
	if(m_LineMesh.m_Vertices.size())
		m_LineMesh.destroy();
}



void CubicCurve::setNumSamples(unsigned int samples)
{
	m_Samples = samples;
	//if(m_Samples > 0)
	//	createLine();
}

int CubicCurve::lookupDistance(float distance) const
{
	for (int i = 1; i < m_DistanceLookup.size()-1; i++)
	{
		if (distance < m_DistanceLookup[i])
		{
			return i - 1;
		}
	}
	return m_DistanceLookup.size() - 3; //@todo check this value. looks sketchy
}

glm::vec3 CubicCurve::getAtDistance(float distance) const
{
	//@todo fix the overshoot + 1 . the curve overflows by some amount
	distance = fmod(distance, 1.0f);
	float curveOffset = distance / m_InverseSpeed; // change domain from 0-1 to curve length
	int leftSample = lookupDistance(curveOffset);
	glm::vec3 pointPos1 = m_LineMesh.m_Vertices[leftSample].m_Position;
	glm::vec3 pointPos2 = m_LineMesh.m_Vertices[leftSample + 1].m_Position;
	float pointDistance1 = abs(m_DistanceLookup[leftSample]);
	float pointDistance2 = abs(m_DistanceLookup[leftSample + 1]);
	curveOffset -= pointDistance1;
	pointDistance2 -= pointDistance1;
	float alpha = curveOffset / pointDistance2; // between 0.0 and 1.0
	//linear interpolation
	return (1 - alpha)*pointPos1 + alpha*pointPos2;
}

void CubicCurve::render() const
{
	const GameWorld &gameWorld = GameWorld::get();
	if(gameWorld.isDebugEnabled() && gameWorld.isDebugTypeEnabled(GameWorld::DebugObject::curvePoint))
	{
		//render points
		Object *curvePoint = gameWorld.getDebugObject(GameWorld::DebugObject::curvePoint);
		for (int i = 0; i < 4; i++)
		{ 
			glm::vec3 position((m_Transform.getMatrix() * glm::translate(glm::mat4(), m_ControlPoints[i]))[3]);
			curvePoint->getTransform().setPosition(position);
			curvePoint->render(gameWorld.getViewMatrix(), gameWorld.getProjectionMatrix());
		}
		//render curve line
		glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(m_Transform.getMatrix())); //overwrite the model matrix that was set by rendering the points

		const ShaderProgram *shader = curvePoint->m_Model->getShaderProgram();
		m_LineMesh.render(shader, GL_LINE_STRIP, false);// false means don't draw elements just arrays
	}
}




void CubicCurve::setControlPoints(glm::mat4x3 controlPoints)
{
	m_ControlPoints = controlPoints;
	//if(m_Samples > 0)
	//	createLine();
}


float CubicCurve::getLength() const
{
	return m_CurveLength;
}
void CubicCurve::init()
{
	if(m_LineMesh.m_Vertices.size())
		m_LineMesh.destroy();

	//we have N samples on a curve that is 0.0 to 1.0 long
	float delta = 1.0f / m_Samples;
	vector<Vertex> linePoints;
	float curveLength = 0;
	glm::vec3 prevPoint = getAtTime(0);
	m_DistanceLookup.push_back(0);
	float currTime = delta;
	while(currTime < 1.0f)
	{
		Vertex currVertex;
		glm::vec3 currPoint = getAtTime(currTime);
		float lineLength = glm::length(currPoint - prevPoint);
		m_CurveLength += lineLength;
		currVertex.m_Position = currPoint;
		//currVertex.m_TexCoord = glm::vec2(0);//dummy tex
		//currVertex.m_Normal = glm::vec3(0,1,0); //dummy normal
		m_DistanceLookup.push_back(m_CurveLength);
		linePoints.push_back(currVertex);
		currTime += delta;
		prevPoint = currPoint;
	}
	//save the inverse velocity which is later used in lookup
	m_InverseSpeed = 1.0f / m_CurveLength;

	m_LineMesh.m_Vertices = linePoints;
	m_LineMesh.m_Material.diffuse = glm::vec4(1,1,0,1);//yellow
	//use the same shader as specified for the control points
	ShaderProgram *shader;
	shader = GameWorld::get().getDebugObject(GameWorld::DebugObject::curvePoint)->m_Model->getShaderProgram();
	m_LineMesh.createVAO(shader);
	m_LineMesh.retrieveMaterialLocations(shader);

}




glm::vec3 CubicCurve::getAtTime(float time) const
{
	time = fmod(time, 1.0f);
	glm::vec3 result;
	if(m_Type == CurveType::Bezier)
	{
		result =  (pow(1.0f - time, 3) * m_ControlPoints[0])
				+ (3.0f * time * pow(1 - time, 2) * m_ControlPoints[1])
				+ (3.0f * pow(time,2) * (1.0f - time) * m_ControlPoints[2])
				+ (pow(time,3) *  m_ControlPoints[3]
		);
	}
	else if(m_Type == CurveType::CatmullRom)
	{
		result =  0.5f *(2.0f*m_ControlPoints[1] 
				+ (-m_ControlPoints[0] + m_ControlPoints[2])*time
				+ (2.0f*m_ControlPoints[0] -5.0f*m_ControlPoints[1] + 4.0f*m_ControlPoints[2] - m_ControlPoints[3]) * pow(time,2)
				+ (-m_ControlPoints[0] + 3.0f*m_ControlPoints[1] - 3.0f*m_ControlPoints[2] + m_ControlPoints[3]) * pow(time,3)
		);
	}

	return result;
}


glm::vec4 CubicCurve::getTimeVector(float time) const
{
	return glm::vec4(pow(time,3), pow(time,2), time, 1);
}

