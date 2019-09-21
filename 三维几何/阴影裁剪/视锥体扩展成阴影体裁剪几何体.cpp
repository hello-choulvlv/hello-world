// 判断两个平面的交线是否是轮廓线   
int TestSilhouette(const Plane& p0, const Plane& p1, const Vector3& dir)
{
    float t0 = p0.m_Normal.Dot(dir);  
    float t1 = p1.m_Normal.Dot(dir);  
    if (t0 * t1 > 0.0f)  
    {
        return 0;
    }

    return t0 > 0.0f ? 1 : 2;
}
// 算法：根据光源方向计算视锥体的轮廓线，由轮廓线和光源方向构造包围平面   
// 包围平面过轮廓线且与光源方向平行   
void CreateLightCullingVolume(Camera* pCamera, const Vector3& lightDir, /*out*/vector<Plane>& cullingPlanes)  
{
    // 12条棱的索引，排列顺序为：近平面顺时针4条棱，   
    // 远平面顺时针4条棱,远近平面之间顺时针4条棱   
    static unsigned short index[] =
    {
        1, 0, 0, 2, 2, 3, 3, 1,
        5, 4, 4, 6, 6, 7, 7, 5,
        1, 5, 0, 4, 2, 6, 3, 7,
    };
  
    // 每条棱都是由两个平面相交构成的,以下是与棱对应的12对平面,   
    // 每条棱在每对平面上的第一个平面上是顺序的，在第二个平面上是逆序的   
    // 例如第一条棱（0，1）（近平面上面的那条棱）在第一个平面FP_Near上是顺序的,（右手系拇指朝向法线）,   
    // 在第二个平面FP_Top上是逆序的   
    static EFrustumPlane planePair[][2] =
    {
        {FP_Near, FP_Top}, {FP_Near, FP_Right}, {FP_Near, FP_Bottom}, {FP_Near, FP_Left},   
        {FP_Top, FP_Far}, {FP_Right, FP_Far}, {FP_Bottom, FP_Far}, {FP_Left, FP_Far},   
        {FP_Top, FP_Left}, {FP_Right, FP_Top}, {FP_Bottom, FP_Right}, {FP_Left, FP_Bottom},  
    };  

    const Vector3* pCorner = pCamera->GetFrustumCorners();  
    // 依次测试12条棱是否为轮廓线   
    for (int i = 0; i < 12; i++)  
    {
        int testSilhouette = TestSilhouette(pCamera->GetPlane(planePair[i][0]), pCamera->GetPlane(planePair[i][1]), lightDir);
        // 是轮廓线
        if (testSilhouette > 0)
        {
            // 如果是第一个平面，边是顺着的，如果是第二个平面，边是逆着的   
            Vector3 edgeDir;  
            if (testSilhouette == 1)  
            {
                edgeDir = pCorner[index[i * 2 + 1]] - pCorner[index[i * 2 + 0]];
            }
            else
            {
                edgeDir = pCorner[index[i * 2 + 0]] - pCorner[index[i * 2 + 1]];  
            }  
            // 由轮廓线构造剔除平面，法线朝里   
            Vector3 normal = lightDir.Cross(edgeDir);
            normal.Normalize();
  
            // 将法线保存起来用于剔除   
            m_LightPlanes.push_back(Plane(normal, pCorner[index[i * 2 + 0]]));
        }
    }

    //视锥平面中朝向光源的也都是剔除平面,更准确的说是背离光源的
    for (int i = 0; i < 6; i++)
    {
        const Plane& plane = pCamera->GetPlane((EFrustumPlane)i);
        if (plane.m_Normal.Dot(lightDir) > 0.0f)
        {
            vector<Plane>.push_back(plane);
        }
    }
}