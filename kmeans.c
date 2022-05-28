#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef struct coordinate coordinate;
struct coordinate{
    double value;
    struct coordinate *next;
};
typedef struct point point;
struct point{
    int d;
    struct coordinate *coordinates;
    int ClusterIndex;

};
typedef struct cluster cluster;
struct cluster{
    struct coordinate *centroid;
    int index ;
    int len;
    double norm;

};

cluster* buildKclusters(int k, point *points){
    cluster *clusters, *cl;
    int i, first;
    coordinate *p_co, *c_co, *tmp, *curr;
    clusters = malloc(sizeof(cluster)*k);
    for(i=0; i<k; i++){
        cl = (cluster *) malloc(sizeof(cluster));
        p_co=points[i].coordinates;
        first=1;
        while(p_co!=NULL){
            if(first){
                c_co=(coordinate *)malloc(sizeof(coordinate));
                c_co->value=p_co->value;
                c_co->next=NULL;
                curr = c_co;
                first=0;
            }
            else{
                tmp = (coordinate*) malloc(sizeof(coordinate));
                if(tmp == NULL){
                    printf("unable to allocate memory");
                    break;
                }
                tmp->value=p_co->value;
                tmp->next=NULL; 
                curr->next=tmp;
                curr=curr->next;
            }
            p_co=p_co->next;
        }
        cl->centroid=c_co;
        cl->index=i;
        cl->len=0;
        clusters[i]=*cl;
    }
    return clusters;
}

void assigncluster(point *points, cluster *clusters, int k, int line){
    double distance, min;
    int index, i,j;
    for( i=0; i<line; i++){
        point *p = &points[i];
        coordinate *pc = (p->coordinates);
        min = -1.0;
        index = -1;
        for( j=0; j<k; j++){
            coordinate *c = (clusters[j].centroid);
            pc = (p->coordinates);
            distance=0;
            while(c!=NULL && pc!=NULL){
                distance=distance + pow((pc->value-c->value),2.0);
                pc=pc->next;
                c=c->next;
            }
            distance = fabs(sqrt(distance));
            if(min<0 || distance<min){
                min = distance;
                index = j;
            }
        }
        p->ClusterIndex=index;
        clusters[index].len+=1;
    }
}
void sumVectors(coordinate *c1, coordinate *c2){
    while(c1!=NULL && c2!=NULL){
       c1->value=c1->value+c2->value;
       c1=c1->next;
       c2=c2->next;
    }
}

void updateCentroid(cluster* clusters, point *points, int line, int k){
    int i,j,len;
    double norma;
    for( i=0; i<k; i++){
        coordinate *centroid = clusters[i].centroid;
        while(centroid!=NULL){
            centroid->value=0;
            centroid=centroid->next;
        }
        for( j=0; j<line; j++){
            if (points[j].ClusterIndex==i)
            {
                sumVectors(clusters[i].centroid, points[j].coordinates);
            }
        }
        len = clusters[i].len;
        centroid=clusters[i].centroid;
        norma=0.0;
        while (centroid!=NULL)
        {
            centroid->value=centroid->value/len;
            norma=norma+pow(centroid->value,2);
            centroid=centroid->next;
        }
        clusters[i].norm=sqrt(norma);
        clusters[i].len=0;

    }
}

int isNumber(char* s)
{
    int i;
    for (i = 0; s[i]!= '\0'; i++)
    {
        if (isdigit(s[i]) == 0)
              return 0;
    }
    return 1;
}


static PyObject* fit(PyObject *self, PyObject *args){
    //k, max_iter, epsilon, size, d, points, clusters
    int k, max_iter, size, d,i,j;
    point *points, *p;
    cluster *clusters, *c;
    coordinate *head, *temp, *curr;
    PyObject *_pythonPoints, *_pythonClusters, *array, *finalClusters, *centroids;
    double epsilon, number; 
    if (!PyArg_ParseTuple(args, "iiiiOOd", &k, &max_iter, &size, &d, &_pythonPoints, &_pythonClusters,&epsilon)) {
        return NULL;
    }
    if (!PyList_Check(_pythonPoints)) {
        return NULL;
    }
    if (!PyList_Check(_pythonClusters)) {
        return NULL;
    }
    points = malloc(sizeof(point) * size);
    clusters = malloc(sizeof(cluster) * k);
    for(i=0; i< size; i++){
        array = PyList_GetItem(_pythonPoints, i);
        for(j=0 ; j<d; j++){
            number = PyFloat_AsDouble(PyList_GetItem(array,j));
            if(j == 0){
            head = (coordinate *)malloc(sizeof(coordinate));
            if(head == NULL){
                printf("An Error Has Occurred");
                return NULL;
            }
            head->value=number;
            head->next=NULL;
            curr=head;
        }
        else{
            temp = (coordinate*) malloc(sizeof(coordinate));
            if(temp == NULL){
                printf("An Error Has Occurred");
                return NULL;
            }
            temp->value=number;
            temp->next=NULL; 
            curr->next=temp;
            curr=curr->next;
        }
        }
        p = malloc(sizeof(point));
        p->d = d;
        p->coordinates = head;
        points[i] = *p;
    }
    for( i=0 ; i<k; i++){
        array = PyList_GetItem(_pythonClusters, i);
        for(j=0 ; j<d; j++){
            number = PyFloat_AsDouble(PyList_GetItem(array,j));
            if(j == 0){
            head = (coordinate *)malloc(sizeof(coordinate));
            if(head == NULL){
                printf("An Error Has Occurred");
                return NULL;
            }
            head->value=number;
            head->next=NULL;
            curr=head;
        }
            else{
                temp = (coordinate*) malloc(sizeof(coordinate));
                if(temp == NULL){
                    printf("An Error Has Occurred");
                    return NULL;
                }
                temp->value=number;
                temp->next=NULL; 
                curr->next=temp;
                curr=curr->next;
        }
        }
        c = malloc(sizeof(cluster));
        c->centroid = head;
        c->index = i; 
        c->len=0;
        clusters[i] = *c;
    }
    clusters = kmeans_pp(k, max_iter, d, size, points, clusters, epsilon);
    finalClusters = PyList_New(k);
    if(finalClusters==NULL){
        return NULL;
    }
    for(i=0; i<k; i++){
        centroids = PyList_New(d);
        if(centroids == NULL){
            return NULL;
        }
        temp = clusters[i].centroid;
        j=0;
        while(temp->next!=NULL){
            PyList_SetItem(centroids,j,Py_BuildValue("d",temp->value));
            temp = temp->next;
            j++; 
        }
        PyList_SetItem(centroids,j,Py_BuildValue("d",temp->value));
        PyList_SetItem(finalClusters,i,Py_BuildValue("O",centroids));

    }
    free(clusters);
    free(points);
    

}


cluster* kmeans_pp(int k, int max_iter, int d, int size, point* points, cluster* clusters, double epsilon){

    int line = size, iter_num = 0, Euclidean_Norm = 1, i;
    double *old_norms;
    coordinate *co;
    point p;
    cluster cl;
    while (iter_num<=max_iter && Euclidean_Norm)
    {
        assigncluster(points,clusters,k,line);
        old_norms = malloc(sizeof(double)*k);
        if(old_norms == NULL){
            printf("An Error Has Occurred");
            return NULL;
        }
        for( i=0; i<k; i++){
            old_norms[i]=clusters[i].norm;
        }
        updateCentroid(clusters,points,line,k);
        Euclidean_Norm=0;
        for ( i = 0; i < k; i++)
        {
            if(fabs(old_norms[i]-clusters[i].norm)>epsilon){
                Euclidean_Norm=1;
            }
        }
        iter_num++;
    }
    free(old_norms);
    return clusters;
}



static PyMethodDef capiMethods[] = {
        { "fit",
                (PyCFunction)fit,
                METH_VARARGS,
                PyDoc_STR("centroids for k clusters")},
        {NULL,NULL,0,NULL}
};

static struct PyModuleDef _moduledef= {
        PyModuleDef_HEAD_INIT,
        "mykmeanssp",
        NULL,
        -1,
        capiMethods
};

PyMODINIT_FUNC
PyInit_mykmeanssp(void)
{
    PyObject *m;
    m=PyModule_Create(&_moduledef);
    if(!m){
        return NULL;
    }
    return m;
}


