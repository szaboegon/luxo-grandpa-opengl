//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Szabó Egon Róbert
// Neptun : DEQGWW
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const float PI=3.141592653589793238;
vec4 quaternion(float angle, vec3 axis) {
    axis = normalize(axis) * sin(angle / 2);
    return vec4(axis.x, axis.y, axis.z, cos(angle / 2));
}

vec4 qmul(vec4 q1, vec4 q2) {
    vec3 d1 =vec3( q1.x,q1.y,q1.z);
    vec3 d2 =vec3(q2.x,q2.y,q2.z);
    vec3 tmp=d2 * q1.w + d1 * q2.w + cross(d1, d2);
    return vec4(tmp.x,tmp.y,tmp.z, q1.w * q2.w - dot(d1, d2));
}

vec3 Rotate(vec3 u, vec4 q) {
    vec4 qinv=vec4(-q.x, -q.y, -q.z, q.w);
    vec4 qr = qmul(qmul(q, vec4(u.x, u.y, u.z, 0)), qinv);
    return vec3(qr.x, qr.y, qr.z);
}

struct Material {
    vec3 ka, kd, ks;
    float  shininess;
    Material(vec3 _kd, vec3 _ks, float _shininess) : ka(_kd * M_PI), kd(_kd), ks(_ks) { shininess = _shininess; }
};

struct Rotatable{
     virtual void rotate(vec3 pivot, vec3 axis, float dt)=0;
};

struct Sphere: public Rotatable  {
    vec3 center;
    float radius;
    Sphere(const vec3& _center, float _radius) {
        center = _center;
        radius = _radius;
    }

    void rotate(vec3 pivot,vec3 axis, float dt){
        axis= normalize(axis);
        vec4 q= quaternion(PI*dt,axis);
        center=Rotate(center-pivot,q)+pivot;
    }
};

struct Plane{
    vec3 normalVec;
    vec3 point;

    Plane(vec3 _normalVec, vec3 _point){
        normalVec=normalize(_normalVec);
        point=_point;
    }
};

struct Cylinder: public Rotatable{
    vec3 center;
    vec3 direction;
    float radius;
    float height;

    Cylinder(vec3 _center,vec3 _direction, float _radius, float _height){
        center=_center;
        direction= normalize(_direction);
        radius=_radius;
        height=_height;
    }

    void rotate(vec3 pivot,vec3 axis, float dt){
        axis= normalize(axis);
        vec4 q= quaternion(PI*dt,axis);
        center=Rotate(center-pivot,q)+pivot;
        direction= Rotate(direction,q);
    }
};

struct Paraboloid: public Rotatable {
    vec3 vertex;
    vec3 direction;
    float height;
    float aParam;
    float bParam;
    vec3 focus;

    Paraboloid(vec3 _vertex,vec3 _direction, float _height, float _aParam, float _bParam){
        vertex=_vertex;
        direction= normalize(_direction);
        height=_height;
        aParam=_aParam;
        bParam=_bParam;
        focus=vertex+0.25*vec3(aParam,aParam,aParam)*direction;
    }

    void rotate(vec3 pivot,vec3 axis, float dt){
        axis= normalize(axis);
        vec4 q= quaternion(PI*dt,axis);
        vertex=Rotate(vertex-pivot,q)+pivot;
        direction= Rotate(direction,q);
        focus=Rotate(focus-pivot,q)+pivot;
    }
};

struct CircleDisk{
    vec3 normalVec;
    vec3 center;
    float radius;

    CircleDisk(vec3 _normalVec, vec3 _center, float _radius){
        normalVec=_normalVec;
        center=_center;
        radius=_radius;
    }
};

class Camera {
public:
    vec3 eye, lookat, right, up;
    float fov;

    void set(vec3 _eye, vec3 _lookat, vec3 vup, float _fov) {
        eye = _eye;
        lookat = _lookat;
        fov=_fov;
        vec3 w = eye - lookat;
        float focus = length(w);
        right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
        up = normalize(cross(w, right)) * focus * tanf(fov / 2);
    }

    void Animate(float dt) {
        eye = vec3((eye.x - lookat.x) * cos(dt) + (eye.z - lookat.z) * sin(dt) + lookat.x,
                   eye.y,
                   -(eye.x - lookat.x) * sin(dt) + (eye.z - lookat.z) * cos(dt) + lookat.z);
        set(eye, lookat, up, fov);
    }
};

struct Light {
    vec3 position;
    vec3 Le, La;
    Light(vec3 pos, vec3 _Le, vec3 _La) {
        position = pos;
        Le = _Le;
        La=_La;
    }
};

class Shader: public GPUProgram{
public:
    void setUniformMaterial(const std::vector<Material*>& materials) {
        char name[256];
        for (unsigned int mat = 0; mat < materials.size(); mat++) {
            sprintf(name, "materials[%d].ka", mat);
            setUniform(materials[mat]->ka,name);

            sprintf(name, "materials[%d].kd", mat);
            setUniform(materials[mat]->kd,name);

            sprintf(name, "materials[%d].ks", mat);
            setUniform(materials[mat]->ks,name);

            sprintf(name, "materials[%d].shininess", mat);
            setUniform(materials[mat]->shininess,name);
        }
    }

    void setUniformLight(const std::vector<Light*>& lights) {
        char name[256];
        for (unsigned int l = 0; l < lights.size(); l++) {
            sprintf(name, "lights[%d].La", l);
            setUniform(lights[l]->La,name);

            sprintf(name, "lights[%d].Le", l);
            setUniform(lights[l]->Le,name);

            sprintf(name, "lights[%d].position", l);
            setUniform(lights[l]->position,name);
        }
    }

    void setUniformCamera(const Camera& camera) {
        setUniform(camera.eye, "wEye");
        setUniform(camera.lookat, "wLookAt");
        setUniform(camera.right, "wRight");
        setUniform(camera.up, "wUp");
    }

    void setUniformSphere(const Sphere *sphere, std::string n) {
        setUniform(sphere->center,n+".center");
        setUniform(sphere->radius,n+".radius");
    }

    void setUniformPlane(const Plane *plane, std::string n) {
        setUniform(plane->normalVec,n+".normalVec");
        setUniform(plane->point,n+".point");
    }

    void setUniformCylinder(const Cylinder *cyl, std::string n){
        setUniform(cyl->center, n+".center");
        setUniform(cyl->direction, n+".direction");
        setUniform(cyl->radius, n+".radius");
        setUniform(cyl->height, n+".height");
    }

    void setUniformParaboloid(const Paraboloid *par, std::string n){
        setUniform(par->vertex, n+".vertex");
        setUniform(par->direction, n+".direction");
        setUniform(par->height, n+".height");
        setUniform(par->aParam, n+".aParam");
        setUniform(par->bParam, n+".bParam");
    }

    void setUniformCircleDisk(const CircleDisk *disk, std::string n){
        setUniform(disk->normalVec, n+".normalVec");
        setUniform(disk->center, n+".center");
        setUniform(disk->radius, n+".radius");
    }
};

class Scene {
    Plane* plane;
    Cylinder* base;
    CircleDisk* baseTop;
    Sphere* sphereJoint1;
    Cylinder* rod1;
    Sphere* sphereJoint2;
    Cylinder* rod2;
    Sphere* sphereJoint3;
    Paraboloid* lampHead;
    Light *lampLight;

    std::vector<Rotatable*> hierarchy;
    std::vector<Light *> lights;
    Camera camera;
    std::vector<Material*> materials;
public:
    void build() {
        vec3 eye = vec3(0, 0, 6), vup = vec3(0, 1, 0), lookat = vec3(0, 0, 0);
        float fov = 45 * M_PI / 180;
        camera.set(eye, lookat, vup, fov);

        Material *floorMaterial=new Material(vec3(35.0f/255.0f, 35.0f/255.0f, 35.0f/255.0f), vec3(0.5, 0.5, 0.5), 150);
        Material *rodMaterial=new Material(vec3(200.0f/255.0f, 75.0f/255.0f, 49.0f/255.0f), vec3(0.5, 0.5, 0.5), 100);
        Material *jointMaterial=new Material(vec3(166.0f/255.0f, 149.0f/255.0f, 86.0f/255.0f), vec3(0.5, 0.5, 0.5), 30);
        materials.push_back(floorMaterial);
        materials.push_back(jointMaterial);
        materials.push_back(rodMaterial);

        vec3 planeCenter=vec3(0,-1,0);
        vec3 baseCenter=planeCenter;
        float jointOffset=0.03;
        vec3 rod1Dir= normalize(vec3(-2,4,-3));
        vec3 rod2Dir=normalize(vec3(3,2,2));
        vec3 lampHeadDir= normalize(vec3(2,0,1.5));

        plane=new Plane(vec3(0,1,0),planeCenter);
        base=new Cylinder(baseCenter,vec3(0,1,0),0.45,0.08);
        baseTop=new CircleDisk(vec3(0,1,0),baseCenter+base->height*base->direction,0.45);
        sphereJoint1=new Sphere(baseCenter+(base->height+jointOffset)*base->direction, 0.06);
        rod1=new Cylinder(sphereJoint1->center+rod1Dir*(sphereJoint1->radius-jointOffset),rod1Dir,0.045,1);
        sphereJoint2=new Sphere(rod1->center+rod1Dir*(rod1->height-jointOffset), 0.06);
        rod2=new Cylinder(sphereJoint2->center+rod2Dir*(sphereJoint2->radius-jointOffset),rod2Dir,0.045,0.8);
        sphereJoint3=new Sphere(rod2->center+rod2Dir*(rod2->height-jointOffset), 0.06);
        lampHead=new Paraboloid(sphereJoint3->center+lampHeadDir*(sphereJoint2->radius-jointOffset),lampHeadDir,0.4,0.5,0.5);

        hierarchy.push_back(sphereJoint1);
        hierarchy.push_back(rod1);
        hierarchy.push_back(sphereJoint2);
        hierarchy.push_back(rod2);
        hierarchy.push_back(sphereJoint3);
        hierarchy.push_back(lampHead);

        Light *pointLight=new Light(vec3(0,3,-3),vec3(40,40,40),vec3(0.12,0.14,0.24));
        lampLight=new Light(lampHead->vertex,vec3(2,2,2),vec3(0,0,0));
        lights.push_back(pointLight);
        lights.push_back(lampLight);

    }

    void setUniform(Shader& shader){
        shader.setUniformPlane(plane,"plane");
        shader.setUniformCylinder(base,"base");
        shader.setUniformCircleDisk(baseTop,"baseTop");
        shader.setUniformSphere(sphereJoint1,"sphereJoint1");
        shader.setUniformCylinder(rod1,"rod1");
        shader.setUniformSphere(sphereJoint2,"sphereJoint2");
        shader.setUniformCylinder(rod2,"rod2");
        shader.setUniformSphere(sphereJoint3,"sphereJoint3");
        shader.setUniformParaboloid(lampHead,"lampHead");
        shader.setUniformMaterial(materials);
        shader.setUniformLight(lights);
        shader.setUniformCamera(camera);
    }

    void Animate(float dt) {
        camera.Animate(dt/2);

       for(int i=0; i<hierarchy.size(); i++){
            hierarchy.at(i)->rotate(sphereJoint1->center,vec3(1,10,0),dt/3);
        }

       for(int i=2; i<hierarchy.size(); i++) {
            hierarchy.at(i)->rotate(sphereJoint2->center, rod1->direction+vec3(0,1,0), dt/3);
        }

        for(int i=4; i<hierarchy.size(); i++){
            hierarchy.at(i)->rotate(sphereJoint3->center,rod2->direction,dt/3);
        }
        lampLight->position=lampHead->focus;
    }
};

Shader shader;
Scene scene;

const char *vertexSource = R"(
    #version 330
    precision highp float;

    uniform vec3 wLookAt, wRight, wUp;
    layout(location = 0) in vec2 cVertexPosition;
    out vec3 p;

    void main() {
        gl_Position = vec4(cVertexPosition, 0, 1);
        p=wLookAt+wRight*cVertexPosition.x+wUp*cVertexPosition.y;
    }
)";

const char *fragmentSource = R"(
    #version 330
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
)";

class FullScreenTexturedQuad {
    unsigned int vao;
public:
    void create(){
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void Draw() {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
};

FullScreenTexturedQuad fullScreenTexturedQuad;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    scene.build();
    fullScreenTexturedQuad.create();
    shader.create(vertexSource, fragmentSource, "fragmentColor");
    shader.Use();
}

void onDisplay() {
    glClearColor(1.0f, 0.5f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    scene.setUniform(shader);
    fullScreenTexturedQuad.Draw();

    glutSwapBuffers();
}
void onKeyboard(unsigned char key, int pX, int pY) {
}
void onKeyboardUp(unsigned char key, int pX, int pY) {
}
void onMouse(int button, int state, int pX, int pY) {
}
void onMouseMotion(int pX, int pY) {
}
void onIdle() {
    scene.Animate(0.01f);
    glutPostRedisplay();
}