/**
 * @file   DisplayEdgeFunctors.h
 * @author Dan R. Lipsa
 * @date  25 Oct. 2010
 *
 * Implementation for functors that display an edge
 */

void edgeRotation (
    G3D::Matrix3* rotation,
    const G3D::Vector3& begin, const G3D::Vector3& end)
{
    using G3D::Vector3;
    Vector3 newZ = end - begin;
    if (newZ.isZero ())
	return;
    newZ = newZ.unit ();
    Vector3 newX, newY;
    newZ.getTangents (newX, newY);
    rotation->setColumn (0, newX);
    rotation->setColumn (1, newY);
    rotation->setColumn (2, newZ);
}
