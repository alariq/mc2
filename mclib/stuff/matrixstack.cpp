//===========================================================================//
// File:	mtrxstk.cc                                                       //
// Contents: Interface specification for the matrix class                    //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

AffinerMatrix4DStack&
	AffinerMatrix4DStack::Concatenate(const AffineMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	AffineMatrix4D *old_top = Peek();
	Check_Object(old_top);
	AffineMatrix4D *new_top = Cast_Pointer(AffineMatrix4D*, MemoryStack::Push());
	new_top->Multiply(matrix, *old_top);
	return *this;
}

AffineMatrix4D&
	AffinerMatrix4DStack::Push(const AffineMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	AffineMatrix4D *new_top = Cast_Pointer(AffineMatrix4D*, MemoryStack::Push());
	return *new_top = matrix;
}

LinearMatrix4DStack&
	LinearMatrix4DStack::Concatenate(const LinearMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	LinearMatrix4D *old_top = Peek();
	Check_Object(old_top);
	LinearMatrix4D *new_top = Cast_Pointer(LinearMatrix4D*, MemoryStack::Push());
	new_top->Multiply(matrix, *old_top);
	return *this;
}

LinearMatrix4D&
	LinearMatrix4DStack::Push(const LinearMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	LinearMatrix4D *new_top = Cast_Pointer(LinearMatrix4D*, MemoryStack::Push());
	return *new_top = matrix;
}

Matrix4DStack&
	Matrix4DStack::Concatenate(const Matrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	Matrix4D *old_top = Peek();
	Check_Object(old_top);
	Matrix4D *new_top = Cast_Pointer(Matrix4D*, MemoryStack::Push());
	new_top->Multiply(matrix, *old_top);
	return *this;
}

Matrix4DStack&
	Matrix4DStack::Concatenate(const AffineMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	Matrix4D *old_top = Peek();
	Check_Object(old_top);
	Matrix4D *new_top = Cast_Pointer(Matrix4D*, MemoryStack::Push());
	new_top->Multiply(matrix, *old_top);
	return *this;
}

Matrix4D&
	Matrix4DStack::Push(const Matrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	Matrix4D *new_top = Cast_Pointer(Matrix4D*, MemoryStack::Push());
	return *new_top = matrix;
}

Matrix4D&
	Matrix4DStack::Push(const AffineMatrix4D& matrix)
{
	Check_Object(this);
	Check_Object(&matrix);

	Matrix4D *new_top = Cast_Pointer(Matrix4D*, MemoryStack::Push());
	return *new_top = matrix;
}

