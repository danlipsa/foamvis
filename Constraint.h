/**
 * @file   Constraint.h
 * @author Dan R. Lipsa
 * @date 5 May 2011
 * 
 * Declaration of the Constraint class
 */
#ifndef __CONSTRAINT_H__
#define __CONSTRAINT_H__

class ExpressionTree;

class Constraint
{
public:
    Constraint ()
    {
    }
    Constraint (ExpressionTree* function);    
    bool HasCenter () const
    {
	return m_center;
    }
    const G3D::Vector3& GetCenter () const
    {
	return *m_center;
    }
    void SetCenter(const G3D::Vector3& center)
    {
	m_center.reset (new G3D::Vector3 (center));
    }
    boost::shared_ptr<ExpressionTree> GetFunction () const
    {
	return m_function;
    }
    

private:
    boost::shared_ptr<ExpressionTree> m_function;
    boost::shared_ptr<G3D::Vector3> m_center;
};


#endif //__CONSTRAINT_H__

// Local Variables:
// mode: c++
// End:
