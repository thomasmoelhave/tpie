// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
//
// Copyright 2018, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>
#include <tpie/tpie.h>
#include <tpie/memory.h>
#include <tpie/serialization2.h>
#include <tpie/btree.h>
#include <iomanip>

using namespace tpie;

struct point_t {
	double x, y;
};

struct point_3d_t {
	double x, y;
	float z;
};

template <typename D>
void unserialize(D & dst, point_3d_t & p) {
    using tpie::unserialize;
    unserialize(dst, p.x);
    unserialize(dst, p.y);
    unserialize(dst, p.z);
}


struct box2 {
	double xmin, xmax, ymin, ymax;
};
	
template <typename T>
using vec = std::vector<T, tpie::allocator<T>>;

/// Polylines
typedef vec<point_t> vector_point_t;
typedef vector_point_t polyline_t;
typedef vector_point_t multipoint_t;

/// Polygons with holes and multilines
typedef vec<vector_point_t> vector_vector_point_t;
typedef vector_vector_point_t polygon_t;
typedef vector_vector_point_t multiline_t;

/// Multipolygons
typedef vec<vector_vector_point_t> vector_vector_vector_point_t;
typedef vector_vector_vector_point_t multipolygon_t;

/// Bezier line
typedef vector_point_t bezierline_t;
typedef vector_vector_point_t bezierpolygon_t;


/// Polylines
typedef vec<point_3d_t> vector_point_3d_t;
typedef vector_point_3d_t polyline_3d_t;
typedef vector_point_3d_t multipoint_3d_t;

/// Polygons with holes and multilines
typedef vec<vector_point_3d_t> vector_vector_point_3d_t;
typedef vector_vector_point_3d_t polygon_3d_t;
typedef vector_vector_point_3d_t multiline_3d_t;

/// Multipolygons
typedef vec<vector_vector_point_3d_t> vector_vector_vector_point_3d_t;
typedef vector_vector_vector_point_3d_t multipolygon_3d_t;

/// Bezier line
typedef vector_point_3d_t bezierline_3d_t;
typedef vector_vector_point_3d_t bezierpolygon_3d_t;

//NOTE Please only add new types at the end so old serializations are not messed up.
enum GeometryType {
	NoGeometryType=0,
	Point=1,
	Polyline=2,
	Polygon=3, /// Polygon with holes
	MultiPolygon=4, /// Polygon with multiple outer rings
	BezierLine=5,
	MultiLine=6, /// Multiple polylines
	BezierPolygon=7,
	Point3D=8,
	Polyline3D=9,
	Polygon3D=10, /// Polygon with holes
	MultiPolygon3D=11, /// Polygon with multiple outer rings
	BezierLine3D=12,
	MultiLine3D=13, /// Multiple polylines
	BezierPolygon3D=14,
	MultiPoint3D=15,
	MultiPoint=16
};

static const size_t NumGeometryTypes=17;


enum FieldValueType {
	IntType=0,
	FloatType=1,
	DoubleType=2,
	StringType=3,
	LongType=4,
};

static const size_t NumFieldValueTypes=5;

class simple_field_t {
public:
	simple_field_t() {}
	simple_field_t(float v): type(FloatType), float_value(v) {}
	simple_field_t(double v): type(DoubleType), double_value(v) {}
	simple_field_t(int v): type(IntType), int_value(v) {}
	simple_field_t(long long v): type(LongType), long_value(v) {}
	simple_field_t(const std::string & v): type(StringType), string_value(v) {}

	FieldValueType type;
	union {
		float float_value;
		int int_value;
		long long long_value;
		double double_value;
	};
	std::string string_value;
};

template <typename S>
void unserialize(S & src, simple_field_t & f) {
	using tpie::unserialize;
	uint8_t t;
	unserialize(src, t);
	switch (t) {
	case IntType: {
		int v;
		unserialize(src, v);
		f = simple_field_t(v);
		break;
	}
	case LongType: {
		long long v;
		unserialize(src, v);
		f = simple_field_t(v);
		break;
	}
	case FloatType: {
		float v;
		unserialize(src, v);
		f = simple_field_t(v);
		break;
	}
	case DoubleType: {
		double v;
		unserialize(src, v);
		f = simple_field_t(v);
		break;
	}
	case StringType: {
		std::string v;
		unserialize(src, v);
		f = simple_field_t(v);
		break;
	}
	default:
		throw std::runtime_error("Bad shape io type");
	}
}


struct field_t: public simple_field_t {
	std::string name;
	field_t() = default;
	field_t(FieldValueType type, std::string name): simple_field_t(type), name(name) {}
};

template <typename Src>
void unserialize(Src & s, field_t & f) {
	using tpie::unserialize;
	unserialize(s, (simple_field_t &)f);
	unserialize(s, f.name);
}

// Vector items

struct vector_item {
	typedef std::vector<simple_field_t> fields_t;
	fields_t fields;
	GeometryType type;
	virtual ~vector_item() {}
};

struct point_vector_item: public vector_item {
	point_t point;
};

struct multipoint_vector_item: public vector_item {
	multipoint_t points;
};

struct polyline_vector_item: public vector_item {
	polyline_t polyline;
};

struct polygon_vector_item: public vector_item {
	polygon_t polygon;
};

struct multipolygon_vector_item: public vector_item {
	multipolygon_t polygon;
};

struct bezierline_vector_item: public vector_item {
	bezierline_t bezierline;
};

struct multiline_vector_item: public vector_item {
	multiline_t lines;
};

struct bezierpolygon_vector_item: public vector_item {
	bezierpolygon_t polygon;
};

struct point_3d_vector_item: public vector_item {
	point_3d_t point;
};

struct multipoint_3d_vector_item: public vector_item {
	multipoint_3d_t points;
};

struct polyline_3d_vector_item: public vector_item {
	polyline_3d_t polyline;
};

struct polygon_3d_vector_item: public vector_item {
	polygon_3d_t polygon;
};

struct multipolygon_3d_vector_item: public vector_item {
	multipolygon_3d_t polygon;
};

struct bezierline_3d_vector_item: public vector_item {
	bezierline_3d_t bezierline;
};

struct multiline_3d_vector_item: public vector_item {
	multiline_3d_t lines;
};

struct bezierpolygon_3d_vector_item: public vector_item {
	bezierpolygon_3d_t polygon;
};



template <typename S>
void unserialize(S & src, point_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.point);
}

template <typename S>
void unserialize(S & src, polyline_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polyline);
}

template <typename S>
void unserialize(S & src, polygon_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, multipolygon_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, bezierline_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.bezierline);
}

template <typename S>
void unserialize(S & src, multiline_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.lines);
}

template <typename S>
void unserialize(S & src, bezierpolygon_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, point_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.point);
}

template <typename S>
void unserialize(S & src, multipoint_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.points);
}

template <typename S>
void unserialize(S & src, polyline_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polyline);
}

template <typename S>
void unserialize(S & src, polygon_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, multipolygon_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, bezierline_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.bezierline);
}

template <typename S>
void unserialize(S & src, multiline_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.lines);
}

template <typename S>
void unserialize(S & src, bezierpolygon_3d_vector_item & i) {
	using tpie::unserialize;
	unserialize(src, i.fields);
	unserialize(src, i.polygon);
}

template <typename S>
void unserialize(S & src, std::shared_ptr<vector_item> & p) {
	using tpie::unserialize;
	uint8_t type;
	tpie::unserialize(src, type);
	switch (type) {
	case Point:
		p=std::make_shared<point_vector_item>();
		unserialize(src, *static_cast<point_vector_item*>(p.get()));
		break;
	case Polyline:
		p=std::make_shared<polyline_vector_item>();
		unserialize(src, *static_cast<polyline_vector_item*>(p.get()));
		break;
	case Polygon:
		p=std::make_shared<polygon_vector_item>();
		unserialize(src, *static_cast<polygon_vector_item*>(p.get()));
		break;
	case MultiPolygon:
		p=std::make_shared<multipolygon_vector_item>();
		unserialize(src, *static_cast<multipolygon_vector_item*>(p.get()));
		break;
	case BezierLine:
		p=std::make_shared<bezierline_vector_item>();
		unserialize(src, *static_cast<bezierline_vector_item*>(p.get()));
		break;
	case MultiLine:
		p=std::make_shared<multiline_vector_item>();
		unserialize(src, *static_cast<multiline_vector_item*>(p.get()));
		break;
	case BezierPolygon:
		p=std::make_shared<bezierpolygon_vector_item>();
		unserialize(src, *static_cast<bezierpolygon_vector_item*>(p.get()));
		break;
	case Point3D:
		p=std::make_shared<point_3d_vector_item>();
		unserialize(src, *static_cast<point_3d_vector_item*>(p.get()));
		break;
	case MultiPoint3D:
		p=std::make_shared<multipoint_3d_vector_item>();
		unserialize(src, *static_cast<multipoint_3d_vector_item*>(p.get()));
		break;
	case Polyline3D:
		p=std::make_shared<polyline_3d_vector_item>();
		unserialize(src, *static_cast<polyline_3d_vector_item*>(p.get()));
		break;
	case Polygon3D:
		p=std::make_shared<polygon_3d_vector_item>();
		unserialize(src, *static_cast<polygon_3d_vector_item*>(p.get()));
		break;
	case MultiPolygon3D:
		p=std::make_shared<multipolygon_3d_vector_item>();
		unserialize(src, *static_cast<multipolygon_3d_vector_item*>(p.get()));
		break;
	case BezierLine3D:
		p=std::make_shared<bezierline_3d_vector_item>();
		unserialize(src, *static_cast<bezierline_3d_vector_item*>(p.get()));
		break;
	case MultiLine3D:
		p=std::make_shared<multiline_3d_vector_item>();
		unserialize(src, *static_cast<multiline_3d_vector_item*>(p.get()));
		break;
	case BezierPolygon3D:
		p=std::make_shared<bezierpolygon_3d_vector_item>();
		unserialize(src, *static_cast<bezierpolygon_3d_vector_item*>(p.get()));
		break;
	default:
		throw std::runtime_error("Bad geometry type");
	}
	p->type=static_cast<GeometryType>(type);
}


void dump(int indent, point_t p) {
	std::cout << std::string(indent, '\t') << p.x << "," << p.y << '\n';
}

void dump(int indent, point_3d_t p) {
	std::cout << std::string(indent, '\t') << p.x << "," << p.y << "," << p.z << '\n';
}

void dump(int indent, vec<point_t> p) {
	if (p.size() < 10) {
		std::cout << std::string(indent, '\t') << '[';
		for (size_t i=0; i < p.size(); ++i) {
			if (i != 0) std::cout << ' ';
			std::cout << p[i].x << "," << p[i].y;
		}
		std::cout << "]\n";
		return;
	}
	std::cout << std::string(indent, '\t') << "[\n";
	std::cout << std::string(indent+1, '\t');
	for (size_t i=0; i < p.size(); ++i) {
		if (i % 11 == 10) {
			std::cout << '\n' << std::string(indent+1, '\t');
		}
		std::cout << p[i].x << "," << p[i].y << ' ';
	}
	std::cout << '\n' << std::string(indent, '\t') << "]\n";
}


void dump(int indent, vec<point_3d_t> p) {
	if (p.size() < 10) {
		std::cout << std::string(indent, '\t') << '[';
		for (size_t i=0; i < p.size(); ++i) {
			if (i != 0) std::cout << ' ';
			std::cout << p[i].x << "," << p[i].y << "," << p[i].z;
		}
		std::cout << "]\n";
		return;
	}
	std::cout << std::string(indent, '\t') << "[\n";
	std::cout << std::string(indent+1, '\t');
	for (size_t i=0; i < p.size(); ++i) {
		if (i % 11 == 10) {
			std::cout << '\n' << std::string(indent+1, '\t');
		}
		std::cout << p[i].x << "," << p[i].y << "," << p[i].z << ' ';
	}
	std::cout << '\n' << std::string(indent, '\t') << "]\n";

}

template <typename T>
void dump(int indent, const vec<T> & i) {
	std::cout << std::string(indent, '\t') << "[\n";
	for (const auto & v: i)
		dump(indent+1, v);
	std::cout << std::string(indent, '\t') << "]\n";
}


inline size_t e_tell(std::basic_fstream<char> & v) {
	return v.tellg();
}

template <typename T>
inline size_t e_tell(T &) {
	return 0; //return static_cast<tpie::serialization_reader*>(&d)->offset();
}

struct Entry {
	uint64_t _;
	std::shared_ptr<vector_item> item;
	
	template<typename S>
	friend void serialize(S &, const Entry &) {
		throw std::runtime_error("Not implemented");
	}
	
	template<typename D>
	friend void unserialize(D & d, Entry & e) {
		using tpie::unserialize;
		unserialize(d, e._);
		unserialize(d, e.item);
	}
};

struct Augmenter {
	template <typename N>
	box2 operator()(const N &) const {
		throw std::runtime_error("Not implemented");
	}
};
	
typedef tpie::btree<
	Entry,
	tpie::btree_augment<Augmenter>,
	tpie::btree_external,
	tpie::btree_serialized,
	tpie::btree_static,
	tpie::btree_unordered,
	tpie::btree_fanout<256, 256>
	> Tree;

struct config {
	bool printHeader = true;
	bool printItems = true;
	bool printFields = false;
	bool printGeometry = false;
	bool recursive = false;
	size_t cnt = 0;
};

void visitItem(const Entry & e, config & c) {
	const auto & i = *e.item;
	if (c.printItems) {
		std::cout << std::setw(5) << c.cnt;
		switch (i.type) {
		case NoGeometryType: std::cout << " NoGeometryType\n"; break;
		case Point: std::cout << " Point\n"; break;
		case Polyline: std::cout << " Polyline\n"; break;
		case Polygon: std::cout << " Polygon\n"; break;
		case MultiPolygon: std::cout << " MultiPolygon\n"; break;
		case BezierLine: std::cout << " BezierLine\n"; break;
		case MultiLine: std::cout << " MultiLine\n"; break;
		case BezierPolygon: std::cout << " BezierPolygon\n"; break;
		case Point3D: std::cout << " Point3D\n"; break;
		case Polyline3D: std::cout << " Polyline3D\n"; break;
		case Polygon3D: std::cout << " Polygon3D\n"; break;
		case MultiPolygon3D: std::cout << " MultiPolygon3D\n"; break;
		case BezierLine3D: std::cout << " BezierLine3D\n"; break;
		case MultiLine3D: std::cout << " MultiLine3D\n"; break;
		case BezierPolygon3D: std::cout << " BezierPolygon3D\n"; break;
		case MultiPoint3D: std::cout << " MultiPoint3D\n"; break;
		case MultiPoint: std::cout << " MultiPoint\n"; break;
		}
	}
	
	if (c.printGeometry) {
		switch (i.type) {
		case NoGeometryType: break;
		case Point: dump(1, reinterpret_cast<const point_vector_item *>(&i)->point); break;
		case Polyline: dump(1, reinterpret_cast<const polyline_vector_item *>(&i)->polyline); break;
		case Polygon: dump(1, reinterpret_cast<const polygon_vector_item *>(&i)->polygon); break;
		case MultiPolygon: dump(1, reinterpret_cast<const multipolygon_vector_item *>(&i)->polygon); break;
		case BezierLine: dump(1, reinterpret_cast<const bezierline_vector_item *>(&i)->bezierline); break;
		case MultiLine: dump(1, reinterpret_cast<const multiline_vector_item *>(&i)->lines); break;
		case BezierPolygon: dump(1, reinterpret_cast<const bezierpolygon_vector_item *>(&i)->polygon); break;
		case Point3D: dump(1, reinterpret_cast<const point_3d_vector_item *>(&i)->point); break;
		case Polyline3D: dump(1, reinterpret_cast<const polyline_3d_vector_item *>(&i)->polyline); break;
		case Polygon3D: dump(1, reinterpret_cast<const polygon_3d_vector_item *>(&i)->polygon); break;
		case MultiPolygon3D: dump(1, reinterpret_cast<const multipolygon_3d_vector_item *>(&i)->polygon); break;
		case BezierLine3D: dump(1, reinterpret_cast<const bezierline_3d_vector_item *>(&i)->bezierline); break;
		case MultiLine3D: dump(1, reinterpret_cast<const multiline_3d_vector_item *>(&i)->lines); break;
		case BezierPolygon3D: dump(1, reinterpret_cast<const bezierpolygon_3d_vector_item *>(&i)->polygon); break;
		case MultiPoint3D: dump(1,reinterpret_cast<const multipoint_3d_vector_item *>(&i)->points); break;
		case MultiPoint: dump(1,reinterpret_cast<const multipoint_vector_item *>(&i)->points); break;
		}
	}
	
	if (c.printFields) {
		std::cout << "\tFields\n";
		size_t fcnt = 0;
		for (const auto & f: i.fields) {
			std::cout << "\t" << std::setw(2) << ++fcnt;
			switch (f.type) {
			case IntType:    std::cout << " Int    " << f.int_value << "\n"; break;
			case FloatType:  std::cout << " Float  " << f.float_value << "\n"; break;
			case DoubleType: std::cout << " Double " << f.double_value << "\n"; break;
			case StringType: std::cout << " String " << f.string_value << "\n"; break;
			case LongType:   std::cout << " Long   " << f.long_value << "\n"; break;
			}
		}
	}
	++c.cnt;
}

template <typename N>
void visitNode(N & node, config & c) {
	const auto children = node.count();
	if (node.is_leaf()) {
		for (size_t i=0; i < children; ++i)
			visitItem(node.value(i), c);
	} else {
		for (size_t i=0; i < children; ++i) {
			node.child(i);
			visitNode(node, c);
			node.parent();
		}
	}
}

int main(int argc, char ** argv) {
	tpie::tpie_init();

	config c;

	int arg = 1;
	while (arg < argc) {
		std::string a(argv[arg]);
		if (a == "--no-items") c.printItems = false;
		else if (a == "--fields") c.printFields = true;
		else if (a == "--geometry") c.printGeometry = true;
		else if (a == "--no-header") c.printHeader = false;
		else if (a == "--recursive") c.recursive = true;
		else break;
		++arg;
	}
	if (arg >= argc || argv[arg][0] == '-') {
		std::cerr << "Usage siv_dump [--no-header] [--no-items] [--fields] [--geometry] file" << std::endl;
		return 1;
	}
	if (!c.printItems) {
		c.printFields = false;
		c.printGeometry = false;
	}		
		
	// Calling tpie_finish() before the three is destructed would result in a segmentation fault. A new scope is created to avoid this.
	{
		tpie::get_memory_manager().set_limit(50*1024*1024);
		Tree tree(argv[arg]);
		if (c.printHeader) std::cout << "======================================> Header <==================================\n";
		
		std::stringstream md(tree.get_metadata());
		uint32_t version;
		unserialize(md, version);
		if (c.printHeader) std::cout << "Version " << version << "\n";

		std::string sref;
		unserialize(md, sref);
		if (c.printHeader) std::cout << "sref " << sref << "\nn";
		
		box2 box;
		unserialize(md, box);
		if (c.printHeader) std::cout << "bbox [" << box.xmin << "," << box.ymin << "," << box.xmax << "," << box.ymax  << "]\n";
		
		stream_size_type numTypes = 0;
		unserialize(md, numTypes);
		if (c.printHeader) std::cout << "Geometry counts (" << numTypes << "):\n";
		for (stream_size_type i=0; i < numTypes; ++i) {
			stream_size_type num = 0;
			unserialize(md, num);
			if (num == 0 || !c.printHeader) continue;
			switch (i) {
			case Point: std::cout << "\tPoint "; break;
			case Polyline: std::cout << "\tPolyline "; break;
			case Polygon: std::cout << "\tPolygon "; break;
			case MultiPolygon: std::cout << "\tMultiPolygon "; break;
			case BezierLine: std::cout << "\tBezierLine "; break;
			case MultiLine: std::cout << "\tMultiLine "; break;
			case BezierPolygon: std::cout << "\tBezierPolygon "; break;
			case Point3D: std::cout << "\tPoint3D "; break;
			case Polyline3D: std::cout << "\tPolyline3D "; break;
			case Polygon3D: std::cout << "\tPolygon3D "; break;
			case MultiPolygon3D: std::cout << "\tMultiPolygon3D "; break;
			case BezierLine3D: std::cout << "\tBezierLine3D "; break;
			case MultiLine3D: std::cout << "\tMultiLine3D "; break;
			case BezierPolygon3D: std::cout << "\tBezierPolygon3D "; break;
			case MultiPoint3D: std::cout << "\tMultiPoint3D "; break;
			case MultiPoint: std::cout << "\tMultiPoint "; break;
			default: std::cout << "\t" <<  i << " "; break;
			}
			std::cout << num << "\n"; 
		}
		
		std::vector<field_t> fields;
		unserialize(md, fields);
		if (c.printHeader) {
			size_t fcnt = 0;
			std::cout << "Geometry fields:\n";
			for (const auto & f: fields) {
				std::cout << "\t" << std::setw(2) << ++fcnt;
				switch (f.type) {
				case IntType:    std::cout << " Int    " << f.name << "\n"; break;
				case FloatType:  std::cout << " Float  " << f.name << "\n"; break;
				case DoubleType: std::cout << " Double " << f.name << "\n"; break;
				case StringType: std::cout << " String " << f.name << "\n"; break;
				case LongType:   std::cout << " Long   " << f.name << "\n"; break;
				}
			}
		}

		if (c.printItems)
			std::cout << "======================================> Items <======================================\n";

		if (c.recursive) {
			auto node = tree.root();
			visitNode(node, c);
		} else {
			for (const auto & e: tree) {
				visitItem(e, c);
			}
		}
	}
	tpie::tpie_finish();
}
