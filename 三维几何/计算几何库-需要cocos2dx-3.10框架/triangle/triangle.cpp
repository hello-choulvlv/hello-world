/*
  *3d������,���μ����㷨
  *2019��7��12��
  *@author:xiaohuaxiong
 */
#include "triangle.h"
#include "matrix/matrix.h"
#include "line/line.h"
#include "cycle_sphere/cycle_sphere.h"
using namespace cocos2d;
NS_GT_BEGIN

bool rectangle_create(Rectangle &rect, const cocos2d::Vec3 &min_vertex, const cocos2d::Vec3 &max_vertex, const cocos2d::Vec3 &axis, float angle)
{
	mat3x3  rotate_matrix;
	mat3_create_rotate(rotate_matrix, normalize(axis), angle);

	rect.center = (min_vertex + max_vertex) * 0.5f;
	rect.extent.x = (max_vertex.x - min_vertex.x) * 0.5f;
	rect.extent.y = (max_vertex.y - min_vertex.y) * 0.5f;

	rect.xaxis.x = rotate_matrix.m[0];
	rect.xaxis.y = rotate_matrix.m[1];
	rect.xaxis.z = rotate_matrix.m[2];

	rect.yaxis.x = rotate_matrix.m[3];
	rect.yaxis.y = rotate_matrix.m[4];
	rect.yaxis.z = rotate_matrix.m[5];

	return min_vertex.z == max_vertex.z;
}

void rectangle_create(Rectangle &rect, const cocos2d::Vec3 &center, const cocos2d::Vec3 &xaxis, const cocos2d::Vec3 &yaxis, const cocos2d::Vec2 &extent)
{
	rect.center = center;
	rect.extent = extent;
	rect.xaxis = xaxis;
	rect.yaxis = yaxis;
}

void rectangle_get_vertex(const Rectangle &rect, cocos2d::Vec3 *vertex)
{
	vertex[0] = rect.center - rect.xaxis * rect.extent.x - rect.yaxis * rect.extent.y;
	vertex[1] = rect.center + rect.xaxis * rect.extent.x - rect.yaxis * rect.extent.y;
	vertex[2] = rect.center + rect.xaxis * rect.extent.x + rect.yaxis * rect.extent.y;
	vertex[3] = rect.center - rect.xaxis * rect.extent.x + rect.yaxis * rect.extent.y;
}

bool rectangle3v_create(Rectangle3v &rect, const cocos2d::Vec3 &corner, const cocos2d::Vec3 &xaxis, const cocos2d::Vec3 &yaxis, const cocos2d::Vec2 &extent)
{
	rect.a = corner;
	rect.b = corner + xaxis * extent.x;
	rect.c = corner + yaxis * extent.y;

	return fabsf(dot(xaxis,yaxis)) <= 0.0001f;
}

void rectangle3v_create(Rectangle3v &rect, const cocos2d::Vec3 &corner,const cocos2d::Vec2 &extent,const cocos2d::Vec3 &axis, float angle)
{
	mat3x3  rotate_matrix;
	mat3_create_rotate(rotate_matrix, normalize(axis), angle);

	rect.a = corner;
	rect.b = corner + (*(Vec3*)rotate_matrix.m) * extent.x;
	rect.c = corner + (*(Vec3 *)&rotate_matrix.m[3]) * extent.y;
}

bool rectangle3v_create(Rectangle3v &rect, const cocos2d::Vec3 &corner, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	rect.a = corner;
	rect.b = b;
	rect.c = c;

	return fabsf(dot(b- corner,c- corner)) <= 0.0001f;
}

void rectangle3v_get_vertex(const Rectangle3v &rect, cocos2d::Vec3 *vertex)
{
	vertex[0] = rect.a;
	vertex[1] = rect.b;
	vertex[2] = rect.b + (rect.c - rect.a);
	vertex[3] = rect.c;
}

float rectangle_point_min_distance(const Rectangle &rect, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point)
{
	Vec3 direction = point - rect.center;
	intersect_point = rect.center;
	intersect_point += rect.xaxis * clampf(-rect.extent.x,rect.extent.x,dot(direction,rect.xaxis));
	intersect_point += rect.yaxis * clampf(-rect.extent.y,rect.extent.y,dot(direction,rect.yaxis));

	return (point - intersect_point).length();
}

float rectangle3v_point_min_distance(const Rectangle3v &rect, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point)
{
	Vec3 ab = rect.b - rect.a;
	Vec3 ac = rect.c - rect.a;
	Vec3 pa = point - rect.a;

	float distance = dot(pa,ab);
	float max_distance = dot(ab,ab);

	intersect_point = rect.a;
	if (distance > max_distance)
		intersect_point += ab;
	else if (distance >= 0)
		intersect_point += ab * (distance/max_distance);

	distance = dot(pa,ac);
	max_distance = dot(ac,ac);
	if (distance > max_distance)
		intersect_point += ac;
	else if (distance > 0)
		intersect_point += ac * (distance/max_distance);

	return (intersect_point - point).length();
}

#pragma mark ---------------------Triangle--------------------------------
void triangle_create(Triangle &triangle,const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	triangle.a = a;
	triangle.b = b;
	triangle.c = c;
}

void triangle_create(Triangle &triangle, const cocos2d::Vec3 *v)
{
	triangle.a = v[0];
	triangle.b = v[1];
	triangle.c = v[2];
}

float triangle_point_min_distance(const Triangle &triangle, const cocos2d::Vec3 &point,cocos2d::Vec3 &intersect_point)
{
	//������ǵ�Voronoi��
	Vec3 ab = triangle.b - triangle.a;
	Vec3 ac = triangle.c - triangle.a;
	Vec3 bc = triangle.c - triangle.b;
	//A
	Vec3 ap = point - triangle.a;
	float tn1 = dot(ap,ab);
	float tn2 = dot(ap,ac);
	if (tn1 <= 0 && tn2 <= 0)
	{
		intersect_point = triangle.a;
		return ap.length();
	}
	//B
	Vec3 bp = point - triangle.b;
	float tn3 = dot(bp,-ab);
	float tn4 = dot(bp,bc);
	if (tn3 <= 0 && tn4 <= 0)
	{
		intersect_point = triangle.b;
		return bp.length();
	}
	//C
	Vec3 cp = point - triangle.c;
	float tn5 = dot(cp,-bc);
	float tn6 = dot(cp,-ac);
	if (tn5 <= 0 && tn6 <= 0)
	{
		intersect_point = triangle.c;
		return cp.length();
	}
	const Vec3 normal = cross(ab,ac);
	//AB
	float  f1 = dot(normal, cross(ap, bp));
	if (f1 <= 0 && tn1 >= 0 && tn3 >= 0)
	{
		intersect_point = triangle.a + ab * (tn1/(tn1 + tn3));
		return (point - intersect_point).length();
	}
	//BC
	float f2 = dot(normal,cross(bp,cp));
	if (f2 <= 0 && tn4 >= 0 && tn5 >= 0)
	{
		intersect_point = triangle.b + bc * (tn4/(tn4 + tn5));
		return (point - intersect_point).length();
	}
	//AC
	float f3 = dot(normal,cross(cp,ap));
	if (f3 <= 0 && tn2 >= 0 && tn6 >= 0)
	{
		intersect_point = triangle.a + ac * (tn2/(tn2 + tn6));
		return (point - intersect_point).length();
	}
	//Center
	float t = f1 + f2 + f3;
	intersect_point = triangle.c * (f1/t) + triangle.a * (f2/t) + triangle.b * (f3/t);
	return (point - intersect_point).length();
}
#pragma mark -----------------------------------Tetrahedron------------------------------------
float plane_point_distance(const cocos2d::Vec3 &p, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c)
{
	return dot(p-a,cross_normalize(b-a,c-a));
}

void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 &a, const cocos2d::Vec3 &b, const cocos2d::Vec3 &c, const cocos2d::Vec3 &d)
{
	tet.a = a;
	tet.b = b;
	tet.c = c;
	tet.d = d;
}

void tetrahedron_create(Tetrahedron &tet, const cocos2d::Vec3 *vertex)
{
	tet.a = vertex[0];
	tet.b = vertex[1];
	tet.c = vertex[2];
	tet.d = vertex[3];
}
#define point_plane_distance(p,a,b,c) dot(p-a,cross(b-a,c-a))
float tetrahedron_point_min_distance(const Tetrahedron &tet, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point)
{
	Vec3 ab = tet.b - tet.a;
	Vec3 ac = tet.c - tet.a;
	Vec3 ad = tet.d - tet.a;
	//�����ж�������ķ���
	Vec3  normal = cross(ab,ac);
	float   f = dot(normal,ad);
	const Vec3 &vA = tet.a;
	const Vec3 &vB = f >= 0 ? tet.b:tet.c;
	const Vec3 &vC = f >= 0 ? tet.c : tet.b;
	const Vec3 &vD = tet.d;
	//
	float distance = FLT_MAX;
	intersect_point = point;
	Triangle triangle;
	Vec3        q;
	//DAB
	if (point_plane_distance(point, vD, vA, vB) >= 0)
	{
		triangle_create(triangle, vD, vA, vB);
		distance = triangle_point_min_distance(triangle,point,intersect_point);
	}
	//DBC
	if (point_plane_distance(point, vD, vB, vC) >= 0)
	{
		triangle_create(triangle, vD,vB,vC);
		float d = triangle_point_min_distance(triangle, point, q);
		if (d < distance)
		{
			distance = d;
			intersect_point = q;
		}
	}
	//DCA
	if (point_plane_distance(point, vD, vC, vA) >= 0)
	{
		triangle_create(triangle, vD, vC, vA);
		float d = triangle_point_min_distance(triangle, point, q);
		if (d < distance)
		{
			distance = d;
			intersect_point = q;
		}
	}
	//ABC
	if (point_plane_distance(point, vA, vC, vB) >= 0)
	{
		triangle_create(triangle, vA, vC, vB);
		float d = triangle_point_min_distance(triangle, point, q);
		if (d < distance)
		{
			distance = d;
			intersect_point = q;
		}
	}
	return (point - intersect_point).length();
}
/*
  *����Voronoi����㷨,
  *�������������һ����14��
  *���㷨Ҳ���������㷨�Ļ���
  *���㷨�����ŵ������м����������
  *���㷨��û�н������̶ȵ��Ż�
 */
float tetrahedron_point_min_distance(const cocos2d::Vec3 *vet, const cocos2d::Vec3 &point, cocos2d::Vec3 &intersect_point)
{
	Vec3 ab = vet[1] - vet[0];
	Vec3 ac = vet[2] - vet[0];
	Vec3 ad = vet[3] - vet[0];
	Vec3 normal = cross_normalize(ab,ac);
	float f = dot(normal,ad);
	//�����η���У��
	const Vec3 &a = vet[0];
	const Vec3 &b = f >= 0 ? vet[1]:vet[2];
	const Vec3 &c = f >= 0 ? vet[2]:vet[1];
	const Vec3 &d = vet[3];
	if (f < 0)
	{
		normal.x = -normal.x;
		normal.y = -normal.y;
		normal.z = -normal.z;

		ab = b - a;
		ac = c - a;
	}
	//A
	Vec3 ap = point - a;

	float f1 = dot(ap,ad);
	float f2 = dot(ap,ab);
	float f3 = dot(ap,ac);
	if (f1 <= 0 && f2 <= 0 && f3 <= 0)
	{
		intersect_point = a;
		return ap.length();
	}
	//B
	Vec3 bp = point - b;
	Vec3 bc = c - b;
	Vec3 db = b - d;
	
	float f4 = -dot(bp,db);
	float f5 = -dot(bp,ab);
	float f6 = dot(bp,bc);
	if (f4 <= 0 && f5 <= 0 && f6 <= 0)
	{
		intersect_point = b;
		return bp.length();
	}
	//C
	Vec3 cp = point - c;
	Vec3 cd = d - c;
	float f7 = -dot(cp,bc);
	float f8 = dot(cp,cd);
	float f9 = -dot(cp,ac);
	if (f7 <= 0 && f8 <= 0 && f9 <= 0)
	{
		intersect_point = c;
		return cp.length();
	}
	//D
	Vec3 dp = point - d;

	float f10 = -dot(dp,ad);
	float f11 = -dot(dp, cd);
	float f12 = dot(dp,db);
	if (f10 <= 0 && f11 <= 0 && f12 <= 0)
	{
		intersect_point = d;
		return dp.length();
	}
	//��������
	//AB,��,��
	const Vec3 normal_abd = cross_normalize(ab,ad);
	const Vec3 normal_abd_ab = cross(ab,normal_abd);
	const Vec3 normal_abc_ab = cross(ab,normal);
	float f21 = dot(normal_abc_ab, ap);
	if (f2 >= 0 && f5 >= 0 && dot(normal_abd_ab, ap) >= 0 && f21 >= 0)
	{
		intersect_point = a + ab * (f2/(f2 + f5));
		return (point - intersect_point).length();
	}
	//BC
	const Vec3 normal_bcd = cross_normalize(bc,-db);
	const Vec3 normal_bcd_bc = cross(bc,normal_bcd);
	const Vec3 normal_abc_bc = cross(bc,normal);
	if (f6 >= 0 && f7 >= 0 && dot(bp, normal_bcd_bc) >= 0 && dot(bp, normal_abc_bc) >= 0)
	{
		intersect_point = b + bc * (f6/(f6 + f7));
		return (point - intersect_point).length();
	}
	//CA
	const Vec3 normal_cad = cross_normalize(-ac,cd);
	const Vec3 normal_cad_ca = cross(-ac,normal_cad);
	const Vec3 normal_abc_ca = cross(-ac,normal);
	float f20 = dot(normal_abc_ca, ap);
	if (f3 >= 0 && f9 >= 0 && dot(normal_cad_ca, ap) >= 0 && f20 >= 0)
	{
		intersect_point = c - ac *(f9/(f3 + f9));
		return (intersect_point - point).length();
	}
	//AD
	const Vec3 normal_abd_da = cross(-ad, normal_abd);
	const Vec3 normal_cad_ad = cross(ad, normal_cad);
	float f14 = dot(ap, normal_abd_da);
	float f18 = dot(ap, normal_cad_ad);
	if (f1 >= 0 && f10 >= 0 && f18 >= 0 && f14 >= 0)
	{
		intersect_point = a + ad *(f1/(f1 + f10));
		return (point - intersect_point).length();
	}
	//BD
	const Vec3 normal_abd_bd = cross(-db,normal_abd);
	const Vec3 normal_bcd_db = cross(db,normal_bcd);
	float f15 = dot(normal_abd_bd, bp);
	float f16 = dot(normal_bcd_db, bp);
	if (f4 >= 0 && f12 >= 0 && f15 >= 0 && f16 >= 0)
	{
		intersect_point = b - db * (f4/(f4 + f12));
		return (point - intersect_point).length();
	}
	//CD
	const Vec3 normal_bcd_cd = cross(cd,normal_bcd);
	const Vec3 normal_cad_cd = cross(-cd,normal_cad);
	float f17 = dot(normal_bcd_cd, cp);
	float f19 = dot(normal_cad_cd, cp);
	if (f8 >= 0 && f11 >= 0 && f17 >= 0 && f19 >= 0)
	{
		intersect_point = c + cd * (f8/(f8 + f11));
		return (point - intersect_point).length();
	}
	////////����///////
	//ABC
	float f13 = dot(ap,normal);
	if (f13 <= 0 && dot(ap,normal_abc_ab) <=0 && dot(bp,normal_abc_bc) <=0 && dot(cp,normal_abc_ca) <=0)
	{
		intersect_point = point - normal * f13;
		return f13;
	}
	//ABD
	float f22 = dot(ap, normal_abd);
	if (f22 >= 0 && dot(ap,normal_abd_ab) <= 0 && dot(bp,normal_abd_bd) <= 0 && dot(dp,normal_abd_da) <=0)
	{
		intersect_point = point - normal_abd * f22;
		return f22;
	}
	//BCD
	float f23 = dot(bp, normal_bcd);
	if (f23 >=0 && dot(bp,normal_bcd_bc) <= 0 && dot(cp,normal_bcd_cd) <= 0 && dot(dp,normal_bcd_db) <= 0)
	{
		intersect_point = point - normal_bcd * f23;
		return f23;
	}
	//CAD
	float f24 = dot(ap,normal_cad);
	if (f24 >=0 && dot(ap,normal_cad_ad) <= 0 && dot(dp,normal_cad_cd) <= 0 && dot(cp, normal_cad_ca) <= 0)
	{
		intersect_point = point - normal_cad * f24;
		return f24;
	}

	intersect_point = point;
	return 0;
}

float triangle_segment_distance(const Triangle &triangle, const cocos2d::Vec3 *vert, cocos2d::Vec3 &triangle_point, cocos2d::Vec3 &segment_point)
{
	//���߶ε������˵���������ƽ���ϵ�ͶӰ��
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 ac = triangle.c - triangle.a;
	const Vec3 bc = triangle.c - triangle.b;
	const Vec3 normal = cross_normalize(ab,ac);
	//////////////////////////////////////////////////////////////////////////
	//�����˵��ͶӰ����,������������������ڲ�
	const Vec3 normal_ab = cross_normalize(ab, normal);
	const Vec3 normal_bc = cross_normalize(bc, normal);
	const Vec3 normal_ca = cross_normalize(normal, ac);
	//
	float length_a = normal.dot(vert[0] - triangle.a);
	float length_b = normal.dot(vert[1] - triangle.a);
	const Vec3 project_a = vert[0] - normal * length_a;
	const Vec3 project_b = vert[1] - normal * length_b;

	//�߶���������ƽ��Ľ���
	Vec3 direction = normalize(vert[0] - vert[1]);
	float f = dot(normal, direction);
	//�߶�û��������ƽ��ƽ��
	if (f != 0)
	{
		const Vec3 intersect_point = vert[0] - direction * (length_a / f);
		//����˵�û�����ӳ�����,�������������ڲ�
		if (length_a * length_b <= 0 && dot(intersect_point - triangle.a, normal_ab) <= 0 && dot(intersect_point - triangle.b, normal_bc) <= 0 && dot(intersect_point - triangle.c, normal_ca) <= 0)
		{
			triangle_point = intersect_point;
			segment_point = intersect_point;
			return 0;
		}
	}
	//�߶ζ˵�0��ƽ���ϵ�ͶӰ��
	Vec3 p1, p2, p3;
	Vec3 q1, q2, q3;
	Segment  segment1 = {
		vert[0],vert[1]
	}, segment2 = {triangle.a,triangle.b};
	//AB
	float f1 = segment_segment_minimum_distance(segment1,segment2, segment_point, triangle_point);

	//BC
	segment_create(segment2,triangle.b, triangle.c);
	float f2 = segment_segment_minimum_distance(segment1,segment2,p1,q1);
	if (f2 < f1)
	{
		f1 = f2;
		triangle_point = q1;
		segment_point = p1;
	}
	//CA
	segment_create(segment2, triangle.c, triangle.a);
	f2 = segment_segment_minimum_distance(segment1, segment2, p1, q1);
	if (f2 < f1)
	{
		f1 = f2;
		triangle_point = q1;
		segment_point = p1;
	}
	
	if (dot(project_a - triangle.a, normal_ab) <= 0 && dot(project_a - triangle.b, normal_bc) <= 0 && dot(project_a - triangle.c, normal_ca) <= 0)
	{
		f2 = fabsf(length_a);
		if (f2 < f1)
		{
			f1 = f2;
			triangle_point = project_a;
			segment_point = vert[0];
		}
	}

	if (dot(project_b - triangle.a, normal_ab) <= 0 && dot(project_b - triangle.b, normal_bc) <= 0 && dot(project_b - triangle.c, normal_ca) <= 0)
	{
		f2 = fabsf(length_b);
		if (f2 < f1)
		{
			f1 = f2;
			triangle_point = project_b;
			segment_point = vert[1];
		}
	}
	return f1;
}

void cone_create(Cone &cone, const cocos2d::Vec3 &top, const cocos2d::Vec3 &normal, float h, float r)
{
	cone.top = top;
	cone.normal = normalize(normal);
	cone.h = h;
	cone.r = r;
}

bool cone_plane_intersect_test(const Cone &cone, const Plane &plane)
{
	//�����Ƿ�׶�εĶ��㴦��ƽ�����һ����ռ�
	float top_distance = plane.distanceTo(cone.top);
	const Vec3 normal = top_distance >= 0?plane.normal: -plane.normal;
	//����λ��׶������ƫ�뷨��normal��Զ�ĵ�
	Vec3  parallel_vec = cross(normal, cone.normal);
	Vec3  vertical_vec = cross_normalize(parallel_vec, cone.normal);
	//���׶�εķ�����ƽ��ƽ��
	if (fabsf(dot(parallel_vec, parallel_vec)) <= 0.001f)
	{
		return top_distance * plane.distanceTo(cone.top + cone.normal * cone.h) <=0;
	}
	const Vec3 vertex = cone.top + cone.normal * cone.h + vertical_vec * cone.r;
	float secondary_distance = plane.distanceTo(vertex);
	return top_distance * secondary_distance <= 0;
}

bool triangle_sphere_intersect_test(const Triangle &triangle, const Sphere &sphere)
{
	//���Ƚ���ƽ�����
	const Vec3 normal = cross_normalize(triangle.b - triangle.a, triangle.c - triangle.a);
	float distance = normal.dot(sphere.center - triangle.a);
	if (distance > sphere.radius)
		return false;
	Vec3 intersect_point;
	distance = triangle_point_min_distance(triangle, sphere.center, intersect_point);
	return distance < sphere.radius;
}

bool triangle_triangle_intersect_test(const Triangle &t1, const Triangle &t2)
{
	//����������֮���Ƿ���
	const Vec3 ab1 = t1.b - t1.a;
	const Vec3 bc1 = t1.c - t1.b;
	const Vec3 ca1 = t1.a - t1.c;
	const Vec3 normal1 = cross_normalize(ab1, bc1);
	//Triangle 2
	const Vec3 ab2 = t2.b - t2.a;
	const Vec3 bc2 = t2.c - t2.b;
	const Vec3 ca2 = t2.a - t2.c;
	const Vec3 normal2 = cross_normalize(ab2, bc2);
	const Vec3 normal = cross(normal1, normal2);
	//��ʱ����,������ƽ��ƽ��
	if (length2(normal) <= 0.001f)
	{
		//һ����Ҫ����6��������,����û�о����Ż�
		//triangle 1 ab
		Vec3 normal_ab = cross(ab1, normal1);
		if (dot(normal_ab, t2.a - t1.a) > 0 && dot(normal_ab, t2.b - t1.a) > 0 && dot(normal_ab, t2.c - t1.a) > 0)
			return false;
		//triangle 1 bc
		Vec3 normal_bc = cross(bc1, normal1);
		if (dot(normal_bc, t2.a - t1.b) > 0 && dot(normal_bc, t2.b - t1.b) > 0 && dot(normal_bc, t2.c - t1.b) > 0)
			return false;
		//triangle 1 ca
		Vec3 normal_ca = cross(t1.a - t1.c, normal1);
		if (dot(normal_ca, t2.a - t1.c) > 0 && dot(normal_ca, t2.b - t1.c) > 0 && dot(normal_ca, t2.c - t1.c) > 0)
			return false;
		/////////////////triangle 2 ab////////////////
		normal_ab = cross(ab2, normal2);
		if (dot(normal_ab, t1.a - t2.a) > 0 && dot(normal_ab, t1.b - t2.a) > 0 && dot(normal_ab, t1.c - t2.a) > 0)
			return false;
		//triangle 1 bc
		normal_bc = cross(bc2, normal2);
		if (dot(normal_bc, t1.a - t2.b) > 0 && dot(normal_bc, t1.b - t2.b) > 0 && dot(normal_bc, t1.c - t2.b) > 0)
			return false;
		//triangle 1 ca
		normal_ca = cross(t2.a - t2.c, normal2);
		if (dot(normal_ca, t1.a - t2.c) > 0 && dot(normal_ca, t1.b - t2.c) > 0 && dot(normal_ca, t1.c - t2.c) > 0)
			return false;
		//ƽ���໥ƽ�е�������ü����,��ض�����
		if (fabsf(dot(normal1, t2.a - t1.a)) > 0.001f)
			return false;
		return true;
	}
	//�����淨�����γɵķ�����
	float f1 = normal1.dot(t2.a - t1.a);
	float f2 = normal1.dot(t2.b - t1.a);
	float f3 = normal1.dot(t2.c - t1.a);
	if (f1 * f2 > 0 && f1 * f3 > 0)
		return false;
	f1 = normal2.dot(t1.a - t2.a);
	f2 = normal2.dot(t1.b - t2.a);
	f3 = normal2.dot(t1.c - t2.a);
	if (f1 * f2 > 0 && f1 * f3 > 0)
		return false;
	//����������ƽ����ֱཻ��
	const Vec3  secondary_normal = cross_normalize(normal, normal1);
	const Vec3  base_point = t1.a + secondary_normal * (dot(t2.a - t1.a, normal2) / dot(normal2, secondary_normal));
	//assert
	//float p1 = normal1.dot(base_point - t1.a);
	//float p2 = normal2.dot(base_point - t2.a);
	//����ֱ��������������֮��ֱ����γɵ��߶�
	Vec3 s[2], t[2];
	int  index_s = 0, index_t = 0;
	//triangle 1
	//ab,�㵽ֱ�ߵ��������
	//Vec3 c_normal = cross(t1.a - base_point, secondary_normal);
	f1 = cross(t1.a - base_point, normal).dot(normal1);
	f2 = cross(t1.b - base_point, normal).dot(normal1);
	if (f1 * f2 <= 0.0f)
	{
		s[0] = t1.a + ab1 * (f1 / (f1 - f2));
		index_s += 1;
	}
	//bc
	f1 = f2;
	f2 = cross(t1.c - base_point, normal).dot(normal1);
	if (f1 * f2 <= 0.0f)
	{
		s[index_s] = t1.b + bc1 *(f1 / (f1 - f2));
		index_s += 1;
	}
	//ca
	f1 = f2;
	f2 = cross(t1.a - base_point, normal).dot(normal1);
	if (index_s < 2 && f1 * f2 <= 0)
	{
		s[index_s] = t1.c + ca1 * (f1 / (f1 - f2));
		index_s += 1;
	}
	//triangle 2
	//ab
	f1 = cross(t2.a - base_point, normal).dot(normal2);
	f2 = cross(t2.b - base_point, normal).dot(normal2);
	if (f1 * f2 <= 0.0f)
	{
		t[0] = t2.a + ab2 * (f1 / (f1 - f2));
		index_t += 1;
	}
	//bc
	f1 = f2;
	f2 = cross(t2.c - base_point, normal).dot(normal2);
	if (f1 * f2 <= 0.0f)
	{
		t[index_t] = t2.b + bc2 * (f1 / (f1 - f2));
		index_t += 1;
	}
	//ca
	f1 = f2;
	f2 = cross(t2.a - base_point, normal).dot(normal2);
	if (index_t < 2 && f1 * f2 <= 0)
	{
		t[index_t] = t2.c + ca2 * (f1 / (f1 - f2));
		index_t += 1;
	}
	//�����������
	float fx = fabsf(normal.x);
	float fy = fabsf(normal.y);
	float fz = fabsf(normal.z);
	int index_l = fx >= fy && fx >= fz?0:(fy >= fx && fy >= fz ? 1:2);

	float l1 = fminf(((float*)&s[0])[index_l],((float*)&s[1])[index_l]);
	float l2 = fmaxf(((float*)&s[0])[index_l], ((float*)&s[1])[index_l]);

	float l3 = fminf(((float*)&t[0])[index_l], ((float*)&t[1])[index_l]);
	float l4 = fmaxf(((float*)&t[0])[index_l], ((float*)&t[1])[index_l]);

	return l1 <= l4 && l2 >= l3;
}
/*
  *���㷨����ʹ�õ�Voronoi��
 */
bool triangle_line_intersect_test(const Triangle &triangle, const Line &line)
{
	//��ֱ����������ƽ��֮��Ľ���,���û�н���,����ζ��ֱ����ƽ��ƽ��
	//��ʱ����Ҫ�����������,�������ƽ��
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 bc = triangle.c - triangle.b;
	const Vec3 normal = cross_normalize(ab, bc);

	//ֱ����ƽ��֮��ļн�
	float f = -dot(normal,line.direction);
	if (fabsf(f) < 0.001f)//ƽ�л��߹���
	{
		float distance = dot(normal,line.start_point - triangle.a);
		//ƽ��
		if (fabsf(distance) >= 0.001f)
			return false;
		//�������,����Ҫ�����������ֱ��֮�����̾���
		//AB
		float f1 = dot(cross(line.direction, triangle.a - line.start_point), normal);
		float f2 = dot(cross(line.direction, triangle.b - line.start_point), normal);
		if (f1 * f2 <= 0)
			return true;
		//BC
		float f3 = dot(cross(line.direction, triangle.c - line.start_point), normal);
		if (f2 * f3 <= 0)
			return true;
		//CA
		if (f3 * f1 <= 0)
			return true;
		return false;
	}
	//�����������
	const Vec3 intersect_point = line.start_point + line.direction * (dot(line.start_point - triangle.a,normal)/f);
	if (dot(intersect_point - triangle.a, cross(ab, normal)) > 0 || dot(intersect_point - triangle.b, cross(bc, normal)) > 0 || dot(intersect_point - triangle.c, cross(triangle.a - triangle.c, normal)) > 0)
		return false;
	return true;
}

bool triangle_segment_intersect_test(const Triangle &triangle, const Segment &segment)
{
	//���ֱ�߷���������ƽ��Ľ���
	const Vec3 ab = triangle.b - triangle.a;
	const Vec3 ac = triangle.c - triangle.a;
	const Vec3 normal = cross_normalize(ab, ac);
	//direction
	const Vec3 fv = segment.final_point - segment.start_point;
	float length = fv.length();
	const Vec3 direction = normalize(fv);
	float f = -dot(direction,normal);
	//����ƽ�У����߹���
	if (fabsf(f) < 0.001f)
	{
		const Vec3 v1 = triangle.a - segment.start_point;
		float distance = -dot(normal,v1);
		//ƽ��
		if (fabsf(distance) >= 0.001f)
			return false;
		//������������Ҫ�������߶�֮��ļ��ι�ϵ
		const Vec3 v2 = triangle.b - segment.start_point;
		const Vec3 v4 = segment.final_point - triangle.b;
		float f1 = dot(normal,cross(direction,v1));
		float f2 = dot(normal,cross(direction,v2));
		float f3 = -dot(normal,cross(ab,v2));
		float f4 = dot(normal,cross(ab,v4));
		if (f1 * f2 <= 0.0f && f3 * f4 <= 0.0f)
			return true;
		//BC
		const Vec3 bc = triangle.c - triangle.b;
		const Vec3 v5 = triangle.c - segment.start_point;
		f1 = f2;
		f2 = dot(normal,cross(direction,v5));
		f3 = dot(normal,cross(bc,segment.start_point - triangle.b));
		f4 = dot(normal,cross(bc,segment.final_point - triangle.b));
		if (f1 * f2 <= 0 && f3 * f4 <= 0)
			return true;
		//CA
		const Vec3 v6 = segment.final_point - triangle.c;
		f1 = f2;
		f2 = dot(normal, cross(direction, triangle.a - segment.start_point));
		f3 = -dot(normal,cross(ac,segment.start_point - triangle.c));
		f4 = -dot(normal,cross(ac,v6));
		if (f1 * f2 <= 0.0f && f3 * f4 <= 0.0f)
			return true;
		//����һ����������߶��������ε��ڲ�
		//���,�Ƿ��߶���ȫ��ƽ���һ��
		const Vec3 normal_ab = cross(ab, normal);
		const Vec3 normal_bc = cross(bc, normal);
		const Vec3 normal_ca = cross(normal, ac);
		if (-dot(normal_ab, v1) <= 0 && dot(normal_ab, v4) <= 0 && dot(normal_bc, v4) <= 0 && -dot(normal_bc, v5) <= 0 && -dot(normal_ca, v1) <= 0 && dot(normal_ca, v6) <= 0)
			return true;
		return false;
	}
	//����ƽ��Ľ���
	float extend = dot(normal, segment.start_point - triangle.a) / f;
	if (extend < 0.0f || extend > length)
		return false;
	const Vec3 intersect_point = segment.start_point + direction * extend;
	//�����AB,CA�ķ���
	const Vec3 normal_ab = cross_normalize(normal,ab);
	float d1 = dot(ac,normal_ab);

	const Vec3 normal_ca = cross_normalize(ac,normal);
	float d2 = dot(ab,normal_ca);

	const Vec3 interpolation = intersect_point - triangle.a;
	float f1 = dot(interpolation,normal_ab)/d1;
	if (f1 < 0.0f || f1 > 1.0f)
		return false;

	float f2 = dot(interpolation,normal_ca)/d2;
	if (f2 < 0.0f || f2 > 1.0f)
		return false;
	return f1 + f2 <= 1.0f;
}
///////////////////////////Cyliner//////////////////////////
void cylinder_create(Cylinder &cylinder, const cocos2d::Vec3 &bottom, const cocos2d::Vec3 &top, float r)
{
	const Vec3 direction = top - bottom;
	cylinder.bottom = bottom;
	cylinder.direction = normalize(direction);
	cylinder.length = direction.length();
	cylinder.r = r;
}

void cylinder_create(Cylinder &cylinder, const cocos2d::Vec3 &bottom, const cocos2d::Vec3 &direction, float length, float r)
{
	cylinder.bottom = bottom;
	cylinder.direction = normalize(direction);
	cylinder.length = length;
	cylinder.r = r;
}

bool cylinder_line_intersect_test(const Cylinder &cylinder, const Line &line)
{
	const Vec3 R = line.start_point - cylinder.bottom;
	float a = dot(line.direction,cylinder.direction);
	float b = dot(R, R);
	float c = dot(R, cylinder.direction);
	float d = dot(line.direction,R);
	//������η���
	float A = 1.0f - a * a;
	float B = d - a * c;
	float r2 = cylinder.r * cylinder.r;
	float C = b - c * c - r2;
	//���ֱ����Բ����ƽ��,��ô���ֱ����Բ����ľ���
	if (fabsf(A) < 0.001f)
	{
		const Vec3 K = cylinder.bottom - line.start_point;
		float  proj = dot(K,line.direction);
		return length2(K) - proj * proj <= r2;
	}
	//�����б�ʽ
	float sqt = B* B - A * C;
	if (sqt < 0.0f)
		return false;
	//�������������
	sqt = sqrtf(sqt);
	float t1 = (-B - sqt)/A;
	float t2 = (-B + sqt)/A;
	//�������������Ƿ�λ������Բ������
	const Vec3 vp1 = line.start_point - cylinder.bottom + line.direction * t1;
	const Vec3 vp2 = line.start_point - cylinder.bottom + line.direction * t2;
	//
	float f1 = dot(vp1,cylinder.direction);
	float f2 = dot(vp2,cylinder.direction);
	return f1 >= 0 && f1 <= cylinder.length || f2 >= 0.0f && f2 <= cylinder.length;
}

bool cylinder_segment_intersect_test(const Cylinder &cylinder, const Segment &segment)
{
	const Vec3 R = segment.start_point - cylinder.bottom;
	Vec3 direction = segment.final_point - segment.start_point;
	float segment_length = direction.length();
	direction *= 1.0f/segment_length;

	float a = dot(direction, cylinder.direction);
	float b = dot(R, R);
	float c = dot(R, cylinder.direction);
	float d = dot(direction, R);
	//����߶α����˵���
	if (c < 0.0f && a < 0.0f)
		return false;
	//����߶α����˶���
	if (c > cylinder.length && a > 0.0f)
		return false;
	//����߶α�����Բ���������
	float s = b - c * c;
	const Vec3 w = cross(cylinder.direction,cross(R,cylinder.direction));
	if (s > cylinder.r * cylinder.r && dot(direction, w) > 0.0f)
		return false;
	//���������Ԫһ�η��̵ĸ�
	float A = 1.0f - a * a;
	float B = d - a * c;
	float r2 = cylinder.r * cylinder.r;
	float C = b - c * c - r2;
	//���ֱ����Բ����ƽ��,��ô���ֱ����Բ����ľ���
	if (fabsf(A) < 0.001f)
	{
		//��ʱ��������Ϊ��һ���жϾͿ��Զ϶�����֮��Ĺ�ϵ
		if (C > 0.0f)
			return false;
		//���ֱ�ߴ��ڵײ�֮��,���߶���֮��,�����
		if (a < 0.0f && (c < 0 || c > cylinder.length + segment_length) || a > 0 && (c > cylinder.length || c < -segment_length))
			return false;
		return true;
	}
	//�����б�ʽ
	float sqt = B* B - A * C;
	if (sqt < 0.0f)
		return false;
	//�������������
	sqt = sqrtf(sqt);
	float t1 = (-B - sqt) / A;
	float t2 = (-B + sqt) / A;
	//�������ͬ����һ����ӳ�����
	if (t1 < 0 && t2 < 0 || t1 > segment_length && t2 > segment_length)
		return false;
	//�������������Ƿ�λ������Բ������
	const Vec3 vp1 = segment.start_point - cylinder.bottom + direction * t1;
	const Vec3 vp2 = segment.start_point - cylinder.bottom + direction * t2;
	//
	bool bt1 = false;
	bool bt2 = false;
	//�жϵ�һ������
	float f1 = dot(vp1, cylinder.direction);
	if (t1 >= 0.0f && t1 <= segment_length)
	{
		if (f1 >= 0.0f && f1 <= cylinder.length)
			return true;
		bt1 = true;
	}

	float f2 = dot(vp2, cylinder.direction);
	if (t2 >= 0.0f && t2 <= segment_length)
	{
		if (f2 >= 0.0f && f2 <= cylinder.length)
			return true;
		bt2 = true;
	}
	//����һ���������,�߶δ���Բ������ڲ�,��ʱ�Ѿ��ж��߶εĻ����ӳ��߱ض���Բ�����ཻ
	//��ô,��Ȼ�������߶ε�һ����Բ������(�п��ܳ���һ���˵���Բ������,һ��Խ������������֮һ)
	const Vec3 T = segment.final_point - cylinder.bottom;
	float l = dot(T,cylinder.direction);
	if (c <= cylinder.length && b - d * d <= r2 || l <= cylinder.length && length2(T) - l * l <=r2)
		return true;
	return false;
}
NS_GT_END