#version 450
precision highp float;

const float PI=3.141592653589793238;
vec4 quaternion(float angle, vec3 axis) {
    axis = normalize(axis) * sin(angle / 2);
    return vec4(axis.x, axis.y, axis.z, cos(angle / 2));
}

vec4 qmul(vec4 q1, vec4 q2) {
    vec3 d1 = q1.xyz, d2 = q2.xyz;
    return vec4(d2 * q1.w + d1 * q2.w + cross(d1, d2),
    q1.w * q2.w - dot(d1, d2));
}

vec3 Rotate(vec3 u, vec4 q) {
    vec4 qinv=vec4(-q.x, -q.y, -q.z, q.w); // conjugate
    vec4 qr = qmul(qmul(q, vec4(u.x, u.y, u.z, 0)), qinv);
    return vec3(qr.x, qr.y, qr.z);
}

struct Material {
    vec3 ka, kd, ks;
    float  shininess;
};

struct Light {
    vec3 position;
    vec3 Le, La;
};

struct Hit {
    float t;
    vec3 position, normal;
    int mat;
};

struct Ray {
    vec3 start, dir;
};

struct Sphere{
    vec3 center;
    float radius;
};

struct Plane{
    vec3 normalVec;
    vec3 point;
};

struct Cylinder{
    vec3 center;
    vec3 direction;
    float radius;
    float height;
};

struct Paraboloid {
    vec3 vertex;
    vec3 direction;
    float height;
    float aParam;
    float bParam;
};

struct CircleDisk{
    vec3 normalVec;
    vec3 center;
    float radius;
};

uniform vec3 wEye;
uniform Light lights[2];
uniform Material materials[3];
uniform Plane plane;
uniform Cylinder base;
uniform CircleDisk baseTop;
uniform Sphere sphereJoint1;
uniform Cylinder rod1;
uniform Sphere sphereJoint2;
uniform Cylinder rod2;
uniform Sphere sphereJoint3;
uniform Paraboloid lampHead;

in vec3 p;
out vec4 fragmentColor;

const float epsilon = 0.0001f;

Hit intersectSphere(const Sphere sphere, const Ray ray) {
    Hit hit;
    hit.t=-1;
    vec3 dist = ray.start - sphere.center;
    float a = dot(ray.dir, ray.dir);
    float b = dot(dist, ray.dir) * 2.0f;
    float c = dot(dist, dist) - sphere.radius * sphere.radius;
    float discr = b * b - 4.0f * a * c;
    if (discr < 0) return hit;
    float sqrt_discr = sqrt(discr);
    float t1 = (-b + sqrt_discr) / 2.0f / a;
    float t2 = (-b - sqrt_discr) / 2.0f / a;
    if (t1 <= 0) return hit;
    hit.t = (t2 > 0) ? t2 : t1;
    hit.position = ray.start + ray.dir * hit.t;
    hit.normal = (hit.position - sphere.center) * (1.0f / sphere.radius);
    hit.mat=1;
    return hit;
}

Hit intersectPlane(const Plane plane, const Ray ray){
    Hit hit;
    hit.t=-1;
    float d1=dot(plane.point-ray.start,plane.normalVec);
    float d2=dot(ray.dir,plane.normalVec);
    hit.t=d1/d2;
    hit.position=ray.start+ray.dir*hit.t;
    hit.normal=plane.normalVec;
    hit.mat=0;
    return hit;
}

Hit intersectCylinder(const Cylinder cyl,const Ray ray){
    Hit hit;
    hit.t=-1;
    vec3 halfway=normalize(cyl.direction+vec3(0,1,0));
    Cylinder rotatedCyl=cyl;
    vec4 q=quaternion(PI,halfway);
    rotatedCyl.direction=Rotate(cyl.direction,q);
    rotatedCyl.center=Rotate(cyl.center,q);
    Ray rotatedRay;
    rotatedRay=ray;
    rotatedRay.start=Rotate(ray.start,q);
    rotatedRay.dir=Rotate(ray.dir,q);
    vec2 dist=vec2(rotatedRay.start.x-rotatedCyl.center.x,rotatedRay.start.z- rotatedCyl.center.z);
    vec2 rayDir=vec2(rotatedRay.dir.x,rotatedRay.dir.z);
    float a = dot(rayDir ,rayDir);
    float b = dot(dist, rayDir) * 2.0f;
    float c = dot(dist, dist) - rotatedCyl.radius * rotatedCyl.radius;
    float discr=b*b-4*a*c;
    if (discr < 0) return hit;
    float t1 = (-b + sqrt(discr)) / 2.0f / a;
    float t2 = (-b - sqrt(discr)) / 2.0f / a;
    vec3 hitPos1=rotatedRay.start + rotatedRay.dir * t1;
    vec3 hitPos2=rotatedRay.start + rotatedRay.dir * t2;
    if (hitPos1.y < rotatedCyl.center.y || hitPos1.y>rotatedCyl.height+rotatedCyl.center.y){
        t1=-1;
    }

    if (hitPos2.y < rotatedCyl.center.y || hitPos2.y>rotatedCyl.height+rotatedCyl.center.y){
        t2=-1;
    }
    if(t1<0.0 && t2<0.0)
    return hit;

    if(t2<0){
        hit.t=t1;
        hit.position=hitPos1;
        hit.normal=vec3(hitPos1.x - rotatedCyl.center.x,0,hitPos1.z - rotatedCyl.center.z);
        hit.normal=normalize(hit.normal);
        hit.mat=2;
    }
    else{
        hit.t=t2;
        hit.position=hitPos2;
        hit.normal=vec3(hitPos2.x - rotatedCyl.center.x,0,hitPos2.z - rotatedCyl.center.z);
        hit.normal=normalize(hit.normal);
        hit.mat=2;
    }
    hit.position=Rotate(hit.position,q);
    hit.normal=Rotate(hit.normal,q);
    return hit;
}

Hit intersectParaboloid(const Paraboloid par, const Ray r){
    Hit hit;
    hit.t=-1;
    vec3 halfway=normalize(par.direction+vec3(0,1,0));
    vec4 q=quaternion(PI,halfway);
    Paraboloid rotatedPar=par;
    rotatedPar.vertex=Rotate(par.vertex-halfway,q)+halfway;
    rotatedPar.direction=Rotate(par.direction,q);
    Ray rotatedRay=r;
    rotatedRay.start=(Rotate(r.start-halfway,q)+halfway);
    rotatedRay.dir=Rotate(r.dir,q);
    vec3 rayOffset=(rotatedRay.start-rotatedPar.vertex);
    vec3 rayOffsetParam=vec3(rayOffset.x/par.aParam,rayOffset.y,rayOffset.z/par.bParam);
    vec2 rayDir=vec2(rotatedRay.dir.x/par.aParam,rotatedRay.dir.z/par.bParam);
    float a = dot(rayDir ,rayDir);
    float b = dot(rayOffsetParam.xz, rayDir) * 2.0f-rotatedRay.dir.y;
    float c = dot(rayOffsetParam.xz, rayOffsetParam.xz) - rayOffset.y;
    float discr=b*b-4*a*c;
    if (discr < 0) return hit;
    float t1 = (-b + sqrt(discr)) / 2.0f / a;
    float t2 = (-b - sqrt(discr)) / 2.0f / a;
    vec3 hitPos1=rayOffset + rotatedRay.dir * t1+rotatedPar.vertex;
    vec3 hitPos2=rayOffset + rotatedRay.dir * t2+rotatedPar.vertex;

    if (hitPos1.y < rotatedPar.vertex.y || hitPos1.y>rotatedPar.height+rotatedPar.vertex.y)
    t1=-1;

    if (hitPos2.y < rotatedPar.vertex.y || hitPos2.y>rotatedPar.height + rotatedPar.vertex.y)
    t2=-1;

    if(t1<0 && t2<0)
    return hit;

    if(t2<0){
        hit.t=t1;
        hit.position=hitPos1;
        hit.normal=vec3(2*(hitPos1.x-rotatedPar.vertex.x)/pow(rotatedPar.aParam,2),-1,2*(hitPos1.z-rotatedPar.vertex.z)/pow(rotatedPar.bParam,2));
        hit.normal=normalize(hit.normal);
        hit.mat=2;
    }
    else{
        hit.t=t2;
        hit.position=hitPos2;
        hit.normal=vec3(2*(hitPos2.x-rotatedPar.vertex.x)/pow(rotatedPar.aParam,2),-1,2*(hitPos2.z-rotatedPar.vertex.z)/pow(rotatedPar.bParam,2));
        hit.normal=normalize(hit.normal);
        hit.mat=2;
    }
    hit.position=Rotate(hit.position,q);
    hit.normal=Rotate(hit.normal,q);
    return hit;

}

Hit intersectCircleDisk(const CircleDisk disk, const Ray ray){
    Hit hit;
    hit.t=-1;
    float d1=dot(disk.center-ray.start,disk.normalVec);
    float d2=dot(ray.dir,disk.normalVec);
    hit.t=d1/d2;
    hit.position=ray.start+ray.dir*hit.t;
    if(length(hit.position-disk.center)>disk.radius)
        hit.t=-1;

    hit.normal=disk.normalVec;
    hit.mat=2;
    return hit;
}

Hit firstIntersect(Ray ray) {
    Hit bestHit;
    bestHit.t=-1;
    Hit hit;

    hit = intersectPlane(plane,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))  bestHit = hit;

    hit = intersectCylinder(base,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectCircleDisk(baseTop,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectSphere(sphereJoint1,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectCylinder(rod1,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectSphere(sphereJoint2,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectCylinder(rod2,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectSphere(sphereJoint3,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    hit = intersectParaboloid(lampHead,ray);
    if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t)) bestHit = hit;

    if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
    return bestHit;
}

bool shadowIntersect(Ray ray, float lightDst) {
    if(intersectPlane(plane,ray).t>0 && intersectPlane(plane,ray).t < lightDst ) return true;
    if(intersectCylinder(base,ray).t>0 && intersectCylinder(base,ray).t < lightDst) return true;
    if(intersectCircleDisk(baseTop,ray).t>0 && intersectCircleDisk(baseTop,ray).t < lightDst) return true;
    if (intersectSphere(sphereJoint1,ray).t > 0 && intersectSphere(sphereJoint1,ray).t < lightDst) return true;
    if(intersectCylinder(rod1,ray).t>0 && intersectCylinder(rod1,ray).t < lightDst) return true;
    if (intersectSphere(sphereJoint2,ray).t > 0 && intersectSphere(sphereJoint2,ray).t < lightDst) return true;
    if(intersectCylinder(rod2,ray).t>0 && intersectCylinder(rod2,ray).t < lightDst) return true;
    if (intersectSphere(sphereJoint3,ray).t > 0 && intersectSphere(sphereJoint3,ray).t < lightDst) return true;
    if(intersectParaboloid(lampHead,ray).t>0 && intersectParaboloid(lampHead,ray).t < lightDst) return true;
    return false;
}

vec3 trace(Ray ray) {
    Hit hit = firstIntersect(ray);
    vec3 outRadiance=vec3(0,0,0);
    if (hit.t < 0) return lights[0].La+lights[1].La;
    for(int i=0; i<2; i++){
        outRadiance += materials[hit.mat].ka * lights[i].La;
        vec3 lightDir=normalize(lights[i].position-hit.position);
        float dst= length(lights[i].position-hit.position);
        Ray shadowRay;
        shadowRay.start=hit.position + hit.normal * epsilon;
        shadowRay.dir=lightDir;
        float cosTheta = dot(hit.normal, lightDir);
        if (cosTheta > 0 && !shadowIntersect(shadowRay, dst)){
            outRadiance = outRadiance + lights[i].Le * materials[hit.mat].kd * cosTheta/pow(dst, 2);
            vec3 halfway = normalize(-ray.dir + lightDir);
            float cosDelta = dot(hit.normal, halfway);
            if (cosDelta > 0) outRadiance = outRadiance + lights[i].Le * materials[hit.mat].ks * pow(cosDelta, materials[hit.mat].shininess)/pow(dst, 2);
        }
    }
    return outRadiance;
}

void main() {
    Ray ray;
    ray.start=wEye;
    ray.dir=normalize(p-wEye);
    fragmentColor = vec4(trace(ray),1);
}