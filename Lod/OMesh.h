#pragma once

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

struct MeshTraits : public OpenMesh::DefaultTraits
{
	EdgeTraits
	{
	  private:
		float error_;
	  public:
		EdgeT() : error_(0.0f) { }
		float error() const { return error_; }
		void  set_error(float _err) { error_ = _err; }
	};

	VertexAttributes(OpenMesh::Attributes::Normal);
	FaceAttributes(OpenMesh::Attributes::Normal);
};

typedef OpenMesh::TriMesh_ArrayKernelT<MeshTraits>  OMesh;