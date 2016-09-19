//===========================================================================//
// File:	rotation.tst                                                     //
// Contents: Test_Assumption code for rotation classes                       //
//---------------------------------------------------------------------------//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#include"stuffheaders.hpp"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ EulerAngles ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
//#############################################################################
//
bool
	EulerAngles::TestClass()
{
	SPEW((GROUP_STUFF_TEST, "Starting EulerAngle test..."));

	const EulerAngles
		a(Identity);
	EulerAngles
		b;
	const EulerAngles
		c(Pi_Over_4,Pi_Over_6,Pi_Over_3);

	Test_Assumption(!a.pitch && !a.yaw && !a.roll);
	Test_Assumption(c.pitch == Pi_Over_4 && c.yaw == Pi_Over_6 && c.roll == Pi_Over_3);

	Test_Assumption(!a);
	b = c;
	Test_Assumption(b == c);
	Test_Assumption(b != a);

	Test_Assumption(b[Y_Axis] == b.yaw);
	Test_Assumption(c[Z_Axis] == c.roll);

	b.Lerp(a,c,0.5f);
	Test_Assumption(b == EulerAngles(Stuff::Lerp(a.pitch,c.pitch,0.5f),Stuff::Lerp(a.yaw,c.yaw,0.5f),Stuff::Lerp(a.roll,c.roll,0.5f)));

	LinearMatrix4D m;
	m.BuildRotation(c);
	b = m;
	Test_Assumption(b == c);

	return true;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ UnitQuaternion ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//
//#############################################################################
//#############################################################################
//
#define NUMBER_OF_MULTIPLICATIONS 30
#include"random.hpp"

class fstream;
bool
	UnitQuaternion::TestClass()
{

	SPEW((GROUP_STUFF_TEST, "Starting UnitQuaternion Test..."));

#if 0
//	ofstream testout("gene.tst",ios::app);        //GY

	Vector3D
		r_vec;
	UnitQuaternion
//		q[2],
		q[NUMBER_OF_MULTIPLICATIONS+1],
		multiplied_q(Identity),
		converted_q;
	LinearMatrix4D
//		lm[2],
		lm[NUMBER_OF_MULTIPLICATIONS+1],
		multiplied_lm(true);
	Scalar  r_float;

//DEBUG_STREAM<<"Starting UnitQuaternion::TestClass()\n";
	for (int k=0; k<TEST_SEQUENCE_LENGTH; ++k)
	{

		for(int i=0; i<=NUMBER_OF_MULTIPLICATIONS; ++i)
		{
		//----------------------
		//Generate random vector
		//----------------------
			for ( int j=0; j<3; ++j)
			{
				r_float = 2.0f*Random - 1.0f;
				r_vec[j] = r_float;
			}
		//----------------------
		//Normalize it
		//----------------------
		//	Tell("Normalize vector\n");
			r_vec.Normalize(r_vec);

		//Generate random angle
			Radian radian(TWO_PI*Random - PI);
		//----------------------
		//Make random quaternion
		//----------------------
		//	Tell("Make random quaternion\n");
			SinCosPair p;
			p = radian;
			UnitQuaternion r_q(
				r_vec.x * p.sine,
				r_vec.y * p.sine,
				r_vec.z * p.sine,
				p.cosine
			);

			Check(&r_q);


			q[i] = r_q;
			lm[i] = r_q;
//     }
		//----------------------
		//Multiply quaternions
		//----------------------
		//	Tell("Multiply quaternions...\n");
//			multiplied_q.Multiply(q[1],q[0]);
			UnitQuaternion previous_q(multiplied_q);
			//Tell("Multiply quaternions.\n");
			multiplied_q.Multiply(q[i],previous_q);
			//Tell("Multiplied quaternions\n");
			Check(&multiplied_q);

			if(i)
			{
		//----------------------
		//Normalize quaternion
		//----------------------
		//	Tell("Normalize quaternion\n");
			multiplied_q.Normalize();
			Check(&multiplied_q);
			}

		//----------------------
		//Multiply matrices
		//----------------------
		//	Tell("Multiply matrices...\n");
//			multiplied_lm.Multiply(lm[0],lm[1]);
			LinearMatrix4D previous_lm(multiplied_lm);
			//Tell("Multiply matrices.\n");
			multiplied_lm.Multiply(previous_lm,lm[i]);
			//Tell("Multiplied matrices\n");

//			if(i)
//DEBUG_STREAM<<"Multiplied "<<i<<" times\n";
		}
//DEBUG_STREAM<<"Multiplied "<<(i-1)<<" times\n";

		//----------------------
		//Convert matrix to quaternian
		//----------------------
		//Tell("Convert matrix to quaternian \n");
		converted_q = multiplied_lm;

		Check(&converted_q);

		//-----------------------
		// Compare results
		//-----------------------
		//Tell("Compare results\n");
		for (i=0;i<4;i++)
		{
			if (!Close_Enough(converted_q[i],
									multiplied_q[i],
									SMALL)//4.91e-3f)//2.62e-3f) //1.9874e-3f)//1.66e-4f)//1.35e-4f)//8.0e-6f) //2.75e-06f)//SMALL)
			)
			{
//DEBUG_STREAM<<"i= "<<i<<" delta= "<<(converted_q[i]-multiplied_q[i])<<endl;
//testout<<"i= "<<i<<" delta= "<<(converted_q[i]-multiplied_q[i])<<endl;

			}
		}
//DEBUG_STREAM<<"SUCCESS at "<<(k+1)<<" tests !!!!!\n";
	}
//	testout.close();
//DEBUG_STREAM<<"UnitQuaternion::TestClass  done !\n";
	//Tell("  UnitQuaternion::TestClass is stubbed out!\n");
#endif
	return false;
}

