#pragma once
#include "stdafx.h"
#include "Mesh.hpp"
#include "SQTTransform.hpp"
#include <glm/glm.hpp>

/**@brief Represents a 3D cubic curve
@details Has logic to render the control points and the line of the curve. 
Can choose type of curve from the allowed types in enum CurveType
For rendering uses the Shader specified in the debug object 'curvePoint' stored in GameWorld
@todo Allow for an arbitrary function getAtTime to be passed. ie. custom type
@todo Allow the curve to skip generating a Curve mesh for drawing
@todo Piecewise curves
@todo Fix assignment bug. operator= currently hidden
@todo Make distance lookup from linear search to binary search
@todo Allow settings for size, color,etc
*/
class CubicCurve
{
public:
	/**@brief The types of curve allowed*/
	enum class CurveType{Bezier, CatmullRom};
	/**@brief A Ctor taking everything necessary to construct the curve*/
	CubicCurve(CurveType type, glm::mat4x3 controlPoints, unsigned int samples);
	/**@brief A Ctor taking the type only. Would have to set the control points and number of samples later */
	CubicCurve(CurveType type);
	CubicCurve();
	virtual ~CubicCurve();
	/**
		@brief Sets the number of sampling frequency along the curve. 
		@details Changing this value cause the curve to recaculate the mesh needed to draw it 
	*/
	void setNumSamples(unsigned int samples);
	/**
		@brief Get the length of the curve. Accuracy dependent on the number of samples	
	*/
	float getLength() const;
	/**
		@brief sets the control points of the curve
		@details Changing this value causes the curve to recaculate the mesh needed to draw it 
	*/
	void setControlPoints(glm::mat4x3 controlPoints); //overwrite if need other behavior
	/**
		@brief Get the position along the curve based on the time travelled from 0.0 to 1.0
	*/
	virtual glm::vec3 getAtTime(float time) const; // 0.0 to 1.0
	/**
		@brief Get the position along the curve based on the distance travelled from 0.0 to 1.0
		@details Uses the linear interpolation from the two closest vertices whose distances are specified in the distance table
	*/
	glm::vec3 getAtDistance(float distance) const; // 0.0 to 1.0

	/**
		@brief Draws the control points and the line of the curve
	*/
	void render() const;

	virtual void init();

public:
	CurveType m_Type;
	SQTTransform m_Transform;

protected:
	CubicCurve& operator=(const CubicCurve &other);
	/**
		@brief Generates a line based on the number of samples
		@details Calculates the curve length from the lines between each samples
		Calculates the inverse speed from the curve length.
		Populates the distance lookup table
	*/
	/**@brief Returns vector (t^3, t^2, t^1, 1)*/
	glm::vec4 getTimeVector(float time) const;
	/**
		@brief Perfoms linear search across the distance vector
	*/
	int lookupDistance(float distance) const;

protected:
	unsigned int m_Samples;
	float m_CurveLength;
	Mesh m_LineMesh; //!<Stores the VAO for drawing the curve
	glm::mat4x3 m_ControlPoints;
	std::vector<float> m_DistanceLookup; //!< For each sample stores the its absolute (i.e not necessarily between 0.0 and 1.0) distance along the curve
	float m_InverseSpeed;//!< Gets us from the 0.0 to 1.0 domain to the curve length domain
};
