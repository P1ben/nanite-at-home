#pragma once

#include "framework.h"
#include "Vertex.h"
#include "VecLib/VecLib.h"

class Face {
public:
	VERTEX_ID a;
	VERTEX_ID b;
	VERTEX_ID c;
	//vec3 normal;
	//bool deleted = false;

//public:
	Face() : a(0), b(0), c(0){}
	Face(uint32_t _a, uint32_t _b, uint32_t _c) : a(_a), b(_b), c(_c) {}
	//Face(uint32_t _a, uint32_t _b, uint32_t _c, vec3 _normal) : a(_a), b(_b), c(_c), normal(_normal) {}

	//void RecalculateNormal(std::vector<Vertex>& vertices) {
	//	this->normal = VecLib::CalculateFaceNormal(vertices[a].position, vertices[b].position, vertices[c].position);
	//}

	bool ContainsId(uint32_t index) {
		if (a == index || b == index || c == index) return true;
		return false;
	}

	static void GetRefsById(uint32_t index, const std::vector<Face>& faces, std::vector<Face*>& references) {
		references.clear();

		for (Face f : faces) {
			if (f.ContainsId(index))
				references.push_back(&f);
		}
	}

	static void UpdateIndices(std::vector<Face>& F, const std::vector<uint32_t>& old_indices, uint32_t new_index) {
		for (auto& f : F) {
			for (auto i : old_indices) {
				//Face old = f;
				f.SwapAny(i, new_index);
				//if (!(old == f)) {
				//	//printf("Face change: (%u, %u, %u) -> (%u, %u, %u)\n", old.a, old.b, old.c, f.a, f.b, f.c);
				//}
			}
		}
	}

	bool operator == (const Face& b) {
		if (this->a == b.a && this->b == b.b && this->c == b.c) return true;
		return false;
	}

	bool operator == (const Face& b) const {
		if (this->a == b.a && this->b == b.b && this->c == b.c) return true;
		return false;
	}

	bool operator < (const Face& b) {
		if (this->a < b.a && this->b < b.b && this->c < b.c) {
			return true;
		}
		return false;
	}

	static bool Contains(const std::vector<Face>& faces, const Face& face) {
		for (auto& f : faces) {
			if (f == face) return true;
		}
		return false;
	}

	static bool Contains(const std::vector<Face*>& faces, const Face& face) {
		for (auto& f : faces) {
			if (*f == face) return true;
		}
		return false;
	}

	bool SearchDuplicate() const {
		if (a == b || a == c || b == c) return true;
		return false;
	}

	void SwapAny(uint32_t old_id, uint32_t new_id) {
		//printf("Face changed: (%d, %d, %d) -> ", a, b, c);
		if (a == old_id) a = new_id;
		if (b == old_id) b = new_id;
		if (c == old_id) c = new_id;
		//printf("(%d, %d, %d)\n", a, b, c);
	}

	std::pair<VERTEX_ID, VERTEX_ID> FirstEdge() const {
		if (a > b) {
			return std::pair<VERTEX_ID, VERTEX_ID>(b, a);
		}
		else {
			return std::pair<VERTEX_ID, VERTEX_ID>(a, b);
		}
	}

	std::pair<VERTEX_ID, VERTEX_ID> SecondEdge() const {
		if (b > c) {
			return std::pair<VERTEX_ID, VERTEX_ID>(c, b);
		}
		else {
			return std::pair<VERTEX_ID, VERTEX_ID>(b, c);
		}
	}

	std::pair<VERTEX_ID, VERTEX_ID> ThirdEdge() const {
		if (a > c) {
			return std::pair<VERTEX_ID, VERTEX_ID>(c, a);
		}
		else {
			return std::pair<VERTEX_ID, VERTEX_ID>(a, c);
		}
	}
};