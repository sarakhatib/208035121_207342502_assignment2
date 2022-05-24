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

int main(int argc, char *argv[]){

    char *inputk, *inputFileName, *outputFileName;
    char c;
    int d=0, line=0, first=1, k, iter_num, Euclidean_Norm, i, max_iter=200;
    double epsilon = 0.001, num, *old_norms;
    coordinate *head, *temp, *curr, *co;
    point *points, p;
    FILE *in_file, *out_file;
    cluster *clusters, cl;
    if(argc<4){
        printf("Invalid Input!");
        return 1;
    }
    if(argc>5){
        printf("Invalid Input!");
        return 1;
    }
    inputk = argv[1];
    k = atoi(inputk);
    if(!isNumber(inputk) || k<1){
        printf("Invalid Input!");
        return 1;
    }
    max_iter = 200;
    if(argc==5){
        char* inputMaxIter = argv[2];
        inputFileName = argv[3];
        outputFileName = argv[4];
        if(!isNumber(inputMaxIter)){
            printf("Invalid Input!");
            return 1;
        }
        max_iter = atoi(inputMaxIter);
        if (max_iter<1)
        {
            printf("Invalid Input!");
            return 1;
        }  
    }
    else{
        inputFileName = argv[2];
        outputFileName = argv[3]; 
    }
    in_file  = fopen(inputFileName, "r");
    out_file = fopen(outputFileName, "w"); 
    if(in_file == NULL || out_file == NULL){
        printf("An Error Has Occurred");
        return 1;
    }
    assert(in_file!=NULL);
    assert(out_file!=NULL);
    while ( fscanf( in_file, "%lf%c", &num, &c ) == 2 ){
        if(first == 1){
            head = (coordinate *)malloc(sizeof(coordinate));
            if(head == NULL){
                printf("An Error Has Occurred");
                return 1;
            }
            head->value=num;
            head->next=NULL;
            curr=head;
            first = 0;
        }
        else{
            temp = (coordinate*) malloc(sizeof(coordinate));
            if(temp == NULL){
                printf("An Error Has Occurred");
                return 1;
            }
            temp->value=num;
            temp->next=NULL; 
            curr->next=temp;
            curr=curr->next;
        }

        if(line==0){
            d++;
        }
        if(c=='\n'){
            p.coordinates=head;
            p.d=d;
            first = 1;
            if(line==0){
                points =malloc(sizeof(point));
                if(points == NULL){
                    printf("An Error Has Occurred");
                    return 1;
                }
                points[0] = p ;
            }
            else{
                points = realloc(points, (line+1)*sizeof(point));
                if(points==NULL){
                    printf("An Error Has Occurred");
                    return 1;
                }
                points[line] = p;
            }
            line++;
        }
    }
    clusters = buildKclusters(k, points); 
    iter_num = 0;
    Euclidean_Norm=1;
    while (iter_num<=max_iter && Euclidean_Norm)
    {
        assigncluster(points,clusters,k,line);
        old_norms = malloc(sizeof(double)*k);
        if(old_norms == NULL){
            printf("An Error Has Occurred");
            return 1;
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
    for( i=0; i<k; i++){
        cl = clusters[i];
        co = cl.centroid;
        while (co!=NULL)
        {
            if(co->next==NULL){
                fprintf(out_file,"%.4f",co->value);
            }
            else{
                fprintf(out_file,"%.4f,",co->value);
            }
            co=co->next;
        }
        fprintf(out_file, "\n");
    }
    fclose(in_file);
    fclose(out_file);
    free(clusters);
    free(head);
    free(points);
    free(temp);
    free(old_norms);
    return 0;
}






