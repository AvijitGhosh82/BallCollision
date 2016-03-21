/* Assignment-8
 * CS29003
 * Name: Avijit Ghosh
 * Roll No: 14CH3FP18
 */

// COMPILE USING: gcc collisiongui.c -lm `sdl-config --cflags --libs`


#include <SDL/SDL.h>
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 640


#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <malloc.h>


#define ArraySize 1000000
#define TIME_LIMIT 1000


void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}


void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:
 
    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
 
    while (x >= y)
    {
        set_pixel(surface, (int)(cx + x), (int)(cy + y), pixel);
        set_pixel(surface, (int)(cx + y), (int)(cy + x), pixel);
 
        if (x != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy + y), pixel);
            set_pixel(surface, (int)(cx + y), (int)(cy - x), pixel);
        }
 
        if (y != 0)
        {
            set_pixel(surface, (int)(cx + x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy + x), pixel);
        }
 
        if (x != 0 && y != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy - x), pixel);
        }
 
        error += y;
        ++y;
        error += y;
 
        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}


void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
    // Note that there is more to altering the bitrate of this 
    // method than just changing this value.  See how pixels are
    // altered at the following web page for tips:
    //   http://www.libsdl.org/intro.en/usingvideo.html
    static const int BPP = 4;
 
    double r = (double)radius;
    double dy;
 
    for (dy = 1; dy <= r; dy += 1.0)
    {
        // This loop is unrolled a bit, only iterating through half of the
        // height of the circle.  The result is used to draw a scan line and
        // its mirror image below it.
 
        // The following formula has been simplified from our original.  We
        // are using half of the width of the circle because we are provided
        // with a center and we need left/right coordinates.
 
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;
 
        // Grab a pointer to the left-most pixel for each half of the circle
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;
 
        for (; x <= cx + dx; x++)
        {
            *(Uint32 *)target_pixel_a = pixel;
            *(Uint32 *)target_pixel_b = pixel;
            target_pixel_a += BPP;
            target_pixel_b += BPP;
        }
    }
}



typedef struct list
{
	double tstamp;
	double x;
	double y;
	struct list* next;
}list;


typedef struct particle{
	int color;
	double radius;
	double rx;
	double ry;
	double vx;
	double vy;
	double mass;
	list* l;
	int count;

}particle;

typedef struct CollisionHeap{
	particle *a;
	particle *b;
	double time_stamp;
	int countA, countB;

}CollisionHeap;


CollisionHeap H[ArraySize];
// H[0].time_stamp=-DBL_MAX;
int Heap_s=0;


void sortedInsert(list** head_ref, double rx, double ry, double ht)
{

    list *current;
    list *new_node=(list *)malloc(sizeof(list));

    new_node->x=rx;
    new_node->y=ry;
    new_node->tstamp=ht;
    new_node->next=NULL;

    /* Special case for the head end */
    if (*head_ref == NULL || (*head_ref)->tstamp >= new_node->tstamp)
    {
        new_node->next = *head_ref;
        *head_ref = new_node;
    }
    else
    {
        /* Locate the node before the point of insertion */
        current = *head_ref;
        while (current->next!=NULL && current->next->tstamp < new_node->tstamp)
        {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}


void printList(list *head, FILE *f)
{
	if(head==NULL)
	{
		return;
	}
    list *temp = head;
    while(temp!= NULL)
    {
        fprintf(f,"%lf\t%lf\t%lf\n", temp->x, temp->y, temp->tstamp);
        // fprintf(f,"%lf %lf\n", temp->x, temp->y);

        temp = temp->next;
    }
    fclose(f);

}


double rnd(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}



particle init()
{
	particle p;
	p.color=rand()%16777216 +1;
	p.radius=rnd(0.02,0.05);
    p.mass=p.radius*100;
	p.rx=rnd(0.0,1.0);
	p.ry=rnd(0.0,1.0);
	p.vx=rnd(-0.5,0.5); 
	p.vy=rnd(-0.5,0.5); 
	p.count=0;
	p.l=NULL;
	return p;

}

void pdet(particle p)
{
	printf("X %lf Y %lf VX %lf VY %lf\n", p.rx, p.ry, p.vx, p.vy);
}

double timeToHit(particle *this, particle *that) {
		if (this == that) return DBL_MAX;
		double dx  = that->rx - this->rx;
		double dy  = that->ry - this->ry;
		double dvx = that->vx - this->vx;
		double dvy = that->vy - this->vy;
		double dvdr = dx*dvx + dy*dvy;
		if (dvdr > 0) return DBL_MAX;
		double dvdv = dvx*dvx + dvy*dvy;
		double drdr = dx*dx + dy*dy;
		double sigma = this->radius + that->radius;
		double d = (dvdr*dvdr) - dvdv * (drdr - sigma*sigma);
		// if (drdr < sigma*sigma) StdOut.println("overlapping particles");
		if (d < 0) return DBL_MAX;
		return -(dvdr + sqrt(d)) / dvdv;
	}

double timeToHitVerticalWall(particle *p) {
		if      (p->vx > 0) return (1.0 - p->rx - p->radius) / p->vx;
		else if (p->vx < 0) return (p->radius - p->rx) / p->vx;  
		else             return DBL_MAX;
	}

double timeToHitHorizontalWall(particle *p) {
		if      (p->vy > 0) return (1.0 - p->ry - p->radius) / p->vy;
		else if (p->vy < 0) return (p->radius - p->ry) / p->vy;  
		else             return DBL_MAX;
	}

void bounceOff(particle *a, particle *b) {
		double dx  = b->rx - a->rx;
		double dy  = b->ry - a->ry;
		double dvx = b->vx - a->vx;
		double dvy = b->vy - a->vy;
		double dvdr = dx*dvx + dy*dvy;             // dv dot dr
		double dist = a->radius + b->radius;   // distance between particle centers at collison

		// normal force F, and in x and y directions
		double F = 2 * a->mass * b->mass * dvdr / ((a->mass + b->mass) * dist);
		double fx = F * dx / dist;
		double fy = F * dy / dist;

		// update velocities according to normal force
		a->vx += fx / a->mass;
		a->vy += fy / a->mass;
		b->vx -= fx / b->mass;
		b->vy -= fy / b->mass;

		// update collision counts
		a->count++;
		b->count++;
	}


void bounceOffVerticalWall(particle *p) {
		p->vx = -p->vx;
		p->count++;
	}

void bounceOffHorizontalWall(particle *p) {
		p->vy = -p->vy;
		p->count++;
	}

double kineticEnergy(particle *p) {
		return 0.5 * p->mass * (p->vx*p->vx + p->vy*p->vy);
	}

void move(particle *p,double dt) {
		p->rx += p->vx * dt;
		p->ry += p->vy * dt;
	}

int count(particle *a)
{
	return a->count;
}


//HEAP


void print_Heap()
{
	int i;
	printf("\nheap size===%d\n",Heap_s);
	for(i=1;i<Heap_s;i++)
	{
		printf("%lf\t",H[i].time_stamp);
	}
}


void Insert(CollisionHeap Element)
{
	int i=0;
	Heap_s++;


	H[Heap_s]=Element;

   int now=Heap_s;


   while(H[now/2].time_stamp > Element.time_stamp)
   {
   		H[now]=H[now/2];
   		now/=2;
   }

   H[now]=Element;

}


CollisionHeap* Initcol(double t, particle *p1, particle *p2)
{
			CollisionHeap *n=(CollisionHeap*)malloc(sizeof(CollisionHeap));
			n->time_stamp = t;
			n->a  = p1;
			n->b  = p2;
			if (p1 != NULL) 
				n->countA = count(p1);
			else           
				n->countA = -1;
			if (p2 != NULL) 
				n->countB = count(p2);
			else           
				n->countB = -1;
			return n;
}

CollisionHeap Extract_Min()
{

        CollisionHeap minElement,lastElement;
        int child,now;
        minElement = H[1];
        lastElement = H[Heap_s--];

        for(now = 1; now*2 <= Heap_s ;now = child)
        {
               
                child = now*2;
           
                if(child != Heap_s && H[child+1].time_stamp < H[child].time_stamp ) 
                {
                        child++;
                }

                if(lastElement.time_stamp > H[child].time_stamp)
                {
                        H[now] = H[child];
                }
                else /* It fits there */
                {
                        break;
                }
        }
        H[now] = lastElement;

        // print_Heap();

        return minElement;
}

int isValid(CollisionHeap A) {
			if (A.a != NULL && count(A.a) != A.countA) return 0;
			if (A.b != NULL && count(A.b) != A.countB) return 0;
			return 1;
		}




//PREDICT

double t  = 0.0; //Logical Clock


void predict(particle *a, double limit, particle *arr, int s) {
        if (a == NULL) return;
        int i;
        // particle-particle collisions
        for (i = 0; i < s; i++) {
            double dt = timeToHit(a,&arr[i]);
            if (t + dt <= limit){
            	CollisionHeap *n=Initcol(t+dt, a, &arr[i]);
                Insert(*n);
                // print_Heap(H);
            }
        }

        // particle-wall collisions
        double dtX = timeToHitVerticalWall(a);
        double dtY = timeToHitHorizontalWall(a);
        if (t + dtX <= limit) {
        		CollisionHeap *n1=Initcol(t + dtX, a, NULL);
        		Insert(*n1);
        		// print_Heap(H);fill_cir
        	}
        if (t + dtY <= limit) {
        		CollisionHeap *n2=Initcol(t + dtY, NULL, a);
        		Insert(*n2);
        		// print_Heap(H);
        }
    }

void redraw(double t, double limit, SDL_Surface *screen, particle *arr, int s) {
        SDL_FillRect(screen, NULL, 0x000000);
        int i;
        for (i = 0; i < s; i++) {
                fill_circle(screen, 640*arr[i].rx, 640*arr[i].ry, 640*arr[i].radius, 0xff000000 + arr[i].color);
        }
        // StdDraw.show(20);
        if (t < limit) {
            double dt=0.003;
            CollisionHeap *n=Initcol(t + dt, NULL, NULL);
            Insert(*n);
        }
    }



int simulate(double limit, particle *arr, int s) {
        
        // initialize PQ with collision events and redraw event



    static const int width = 640;
    static const int height = 640;
 
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return 1;
 
    atexit(SDL_Quit);
 
    SDL_Surface *screen = SDL_SetVideoMode(width, height, 0, SDL_DOUBLEBUF);
 
    if (screen == NULL)
        return 2;
    
        // int dum=0;
        // for(dum=0;dum<=90000000;dum++);


        // SDL_FillRect(screen, NULL, 0x000000);
        
        Heap_s=0;

        int i;
        for (i = 0; i < s; i++) {
            predict(&arr[i], limit, arr, s);
        }
        CollisionHeap *n=Initcol(0, NULL, NULL);
        Insert(*n);  
        printf("\nRedraw\n");     // redraw event


        // the main event-driven simulation loop
        while (Heap_s!=1) { 

            // get impending event, discard if invalidated
            CollisionHeap min=Extract_Min();
            if (!isValid(min)) continue;
            particle *a = min.a;
            particle *b = min.b;

            // double ht=min.time_stamp;

            // physical collision, so update positions, and then simulation clock
            for (i = 0; i < s; i++)
                move(&arr[i], min.time_stamp - t);
            t = min.time_stamp;
            if(t>0)
            printf("System at time:  %lf s\n", t);


            SDL_Event event; 
            while(SDL_PollEvent(&event))
            {
                if(event.type == SDL_QUIT)
                    return 0;
            }
     
            SDL_LockSurface(screen);


            // process event
            if(a != NULL && b != NULL) 
            {
            	sortedInsert(&(a->l),a->rx,a->ry,t);
            	sortedInsert(&(b->l),b->rx,b->ry,t);



                // fill_circle(screen, 640*a->rx, 640*a->ry, 640*a->radius, 0xff000000 + a->color);
                // fill_circle(screen, 640*b->rx, 640*b->ry, 640*b->radius, 0xff000000 + b->color);



            	bounceOff(a,b);              // particle-particle collision
            }
            else if (a != NULL && b == NULL) 
            {
            	sortedInsert(&(a->l),a->rx,a->ry,t);

                // fill_circle(screen, 640*a->rx, 640*a->ry, 640*a->radius, 0xff000000 + a->color);

            	bounceOffVerticalWall(a);   // particle-wall collision
            }
            else if (a == NULL && b != NULL) 
            {
            	sortedInsert(&(b->l),b->rx,b->ry,t);

                // fill_circle(screen, 640*b->rx, 640*b->ry, 640*b->radius, 0xff000000 + b->color);

            	bounceOffHorizontalWall(b);
            } // particle-wall collision
            else if (a == NULL && b == NULL) 
                redraw(t,limit,screen,arr,s);
                
                //redraw(limit);               // redraw event

            // update the priority queue with new collisions involving a or b
            predict(a, limit, arr, s);
            predict(b, limit, arr, s);

            SDL_FreeSurface(screen);
     
            SDL_Flip(screen);
        }
    }


int main()
{
	srand(time(NULL));

	H[0].time_stamp=-DBL_MAX;
	printf("Enter number of balls: \n");
	int n;
	scanf("%d", &n);
	particle arr[n];
	int i;
	for(i=0;i<n;i++)
	{
		arr[i]=init();
		pdet(arr[i]);
	}

	simulate(TIME_LIMIT,arr,n);

    return 0;

}



