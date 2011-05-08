/**
 * @file   Constraint.cpp
 * @author Dan R. Lipsa
 * @date 8 May 2011
 * 
 * Implementation of the Constraint class
 */

#include "Constraint.h"
#include "ExpressionTree.h"

Constraint::Constraint (ExpressionTree* function) :
    m_function (function)
{
}

