/**
 * @file   VectorLessThan.h
 * @author Dan R. Lipsa
 * @date 25 March 2010
 * Comparison functors between vectors
 */

class VectorLessThanAngle
{
public:
    VectorLessThanAngle (const G3D::Vector3& originNormal) : 
    m_originNormal (originNormal) {}
    bool operator () (
	const G3D::Vector3& first, const G3D::Vector3& second) const;
    static double angle (
	const G3D::Vector3& first, const G3D::Vector3& second);
    static double angle0pi (
	const G3D::Vector3& first, const G3D::Vector3& second)
    {
	return acos (first.dot (second));
    }
private:
    G3D::Vector3 m_originNormal;
};
