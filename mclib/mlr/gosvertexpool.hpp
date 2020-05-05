//===========================================================================//
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
//===========================================================================//

#pragma once
#define MLR_GOSVERTEXPOOL_HPP

#include<mlr/mlr.hpp>
#include<mlr/gosvertex.hpp>
#include<mlr/gosvertex2uv.hpp>

extern bool MLRVertexLimitReached;

namespace MidLevelRenderer {

	//##########################################################################
	//######################    GOSVertexPool    ###############################
	//##########################################################################

	class GOSVertexPool
	{
		public:
			GOSVertexPool();
			
			void Reset();

//
//------------------------------------------------------------------------------------------------------------
//
			int GetLength () 
				{ 
					Check_Object(this); 
					return (int)vertices.GetLength()-1; 
				}

			unsigned GetLast () 
				{ 
					Check_Object(this); 
					return lastUsed; 
				}

			int Increase (int add=1)
				{ 
					Check_Object(this); 
					lastUsed += add; 
					Verify(lastUsed<Limits::Max_Number_Vertices_Per_Frame);
					MLRVertexLimitReached = (lastUsed > (Limits::Max_Number_Vertices_Per_Frame - 2000));
					return lastUsed;
				}

			GOSVertex*
				GetActualVertexPool(bool db=false)
					{ 
						Check_Object(this); 

						if(db)
						{
							return verticesDB.GetData(); 
						}
						else
						{
							return (GOSVertex*)((char*)(vertices.GetData() + lastUsed)+vertexAlignment); 
						}
					}

//
//------------------------------------------------------------------------------------------------------------
//
			int GetLength2UV () 
				{ 
					Check_Object(this); 
					return (int)vertices2uv.GetLength()-1; 
				}

			unsigned GetLast2UV () 
				{ 
					Check_Object(this); 
					return lastUsed2uv; 
				}

			int Increase2UV (int add=1)
				{ 
					Check_Object(this); 
					lastUsed2uv += add; 
					Verify(lastUsed2uv<Limits::Max_Number_Vertices_Per_Frame);
					return lastUsed2uv;
				}

			GOSVertex2UV*
				GetActualVertexPool2UV(bool db=false)
					{ 
						Check_Object(this); 
						if(db)
						{
							return vertices2uvDB.GetData(); 
						}
						else
						{
							return (GOSVertex2UV*)((char*)(vertices2uv.GetData() + lastUsed2uv)+vertexAlignment); 
						}
					}

//
//------------------------------------------------------------------------------------------------------------
//
			unsigned GetLastIndex () 
				{ 
					Check_Object(this); 
					return lastUsedIndex; 
				}

			int IncreaseIndex (int add=1)
				{ 
					Check_Object(this); 
					lastUsedIndex += add; 
					Verify(lastUsedIndex<Limits::Max_Number_Vertices_Per_Frame);
					return lastUsedIndex;
				}

			unsigned short*
				GetActualIndexPool(bool db=false)
					{ 
						Check_Object(this); 
						if(db)
						{
							return indicesDB.GetData(); 
						}
						else
						{
							return (unsigned short*)((char*)(indices.GetData() + lastUsedIndex)+indicesAlignment); 
						}
					}

//
//------------------------------------------------------------------------------------------------------------
//
			void
				TestInstance()
					{
						Verify(lastUsed < Limits::Max_Number_Vertices_Per_Frame);
						Verify(lastUsed2uv < Limits::Max_Number_Vertices_Per_Frame);
						Verify(lastUsedIndex < Limits::Max_Number_Vertices_Per_Frame);
					}

		protected:
			unsigned lastUsed;
			unsigned lastUsed2uv;
			unsigned lastUsedIndex;
			unsigned vertexAlignment;
			unsigned vertexAlignment2uv;
			unsigned indicesAlignment;

			Stuff::DynamicArrayOf<GOSVertex> vertices; // , Max_Number_Vertices_Per_Frame+4*Max_Number_ScreenQuads_Per_Frame
			Stuff::DynamicArrayOf<GOSVertex2UV> vertices2uv; // , Max_Number_Vertices_Per_Frame+4*Max_Number_ScreenQuads_Per_Frame
			Stuff::DynamicArrayOf<unsigned short> indices; // , Max_Number_Vertices_Per_Frame

			Stuff::DynamicArrayOf<GOSVertex> verticesDB; // , Max_Number_Vertices_Per_Mesh
			Stuff::DynamicArrayOf<GOSVertex2UV> vertices2uvDB; // , Max_Number_Vertices_Per_Mesh
			Stuff::DynamicArrayOf<unsigned short> indicesDB; // , Max_Number_Vertices_Per_Mesh

		private:
			GOSVertexPool(const GOSVertexPool&);
	};

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	inline
		GOSVertexPool::GOSVertexPool() 
	{ 
		Verify(gos_GetCurrentHeap() == Heap);
		lastUsed = 0;
		lastUsed2uv = 0;
		lastUsedIndex = 0;
		
		gos_PushCurrentHeap(StaticHeap);
		vertices.SetLength(Limits::Max_Number_Vertices_Per_Frame+4*Limits::Max_Number_ScreenQuads_Per_Frame+1);
		vertices2uv.SetLength(Limits::Max_Number_Vertices_Per_Frame+4*Limits::Max_Number_ScreenQuads_Per_Frame+1);
		indices.SetLength(Limits::Max_Number_Vertices_Per_Frame+16);

		verticesDB.SetLength(2*Limits::Max_Number_Vertices_Per_Mesh);
		vertices2uvDB.SetLength(2*Limits::Max_Number_Vertices_Per_Mesh);
		indicesDB.SetLength(2*Limits::Max_Number_Vertices_Per_Mesh);

		gos_PopCurrentHeap();

        //sebi !NB
        // TODO: add alignment method to DynamicArray
		vertexAlignment=32-( (size_t)((void*)vertices.GetData()) & 31 );
		vertexAlignment2uv=32-( (size_t)((void*)vertices2uv.GetData()) & 31 );
		indicesAlignment=32-( (size_t)((void*)indices.GetData()) & 31 );
	}


	inline void
		GOSVertexPool::Reset()
	{
		Check_Object(this);
		lastUsed = 0;
		lastUsed2uv = 0;
		lastUsedIndex = 0;
		MLRVertexLimitReached = false;
	}

}
