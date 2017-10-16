/*
  Two tasks fulfilled here:
  * Segmentation on 2D image for key slice detection.
  * Keypoint add/removal between adjacent keyslices.

  Requirement for KSD:
  *. for pillar, use the same boundary to extrude from bottom to top.
  *. for roof, leave window along, extrude part above window from left to right.
  *. for CooperUnion, extrude the walls between windows.
  *. for low resolution, reconstruct the major structure, such as wall+arch (but no window details).

  Failure cases:
  *. Non vertical wall/structure in the vertical direction.
     (because the reconstruction is based on keyslices, extrusion has no way to handle non-vertical structure except
     the special taper structure)
  
*/

#include "project.h"

#ifndef LINUX
////// Graph Cut header

//#define TEST

#ifdef TEST
#define ZOOM 5
#else
#define ZOOM 1
#endif

#define INF 100000000
#define diff(img, x1, y1, x2, y2) pow(img(x1,y1) - img(x2,y2),2)

#ifdef TEST
int VOIS = 2;
#else
int VOIS = 4; // 10
#endif


int BOUNDARY_BALLOONING = 0;
int UNIFORM_BALLOONING = 1;
int FORCE_BALLOONING = 2;


typedef Graph<double,double,double> GraphType;

/**
* Affiche de l'image à l'écran
**/
template <class T> 
void display_image(CImg<T> &image, int delay = 0)
{
	CImgDisplay display(image, "Image");
	if (delay != 0)
		display.wait(delay);
	else
		display.wait();
}

inline int round(double x)
{
	int fl = (int) floor(x);
	if (x-fl > 0.5)
		return fl+1;
	else
		return fl;
}


void compute_sigmas(CImg<double> &image, CImg<double> &sigmas)
{
	int w=image.width;
	int h=image.height;

	for (int i=0; i<w; ++i)
	{
		for (int j=0; j<h; ++j)
		{
			double sum=0;
			int xmax = min(w-1, i+VOIS);
			int ymax = min(h-1, j+VOIS);
			int xmin = max(0,i-VOIS);
			int ymin = max(0,j-VOIS);

			for (int x=xmin; x<xmax; ++x)
			{
				for (int y=ymin; y<ymax; ++y)
				{
					double tmp = image(x,y);

					sum += abs(tmp-image(x+1,y));
					sum += abs(tmp-image(x,y+1));
				}
			}

			sigmas(i,j) = sum / (2*(xmax-xmin)*(ymax-ymin));

		}
	}

}



/**
* Find object points and background points in the mask image
**/
void find_points(CImg<double> &mask,
				 std::vector<Point1> &object_points,
				 std::vector<Point1> &background_points,
				 Point1 &star_point)
{
	for (unsigned int i=0; i<mask.width; ++i)
	{
		for (unsigned int j=0; j<mask.height; ++j)
		{
			double val = mask(i,j);
			if (val == 1)
				object_points.push_back(Point1(i,j));
			else if (val == 0)
				background_points.push_back(Point1(i,j));
			else if (val == 2)
				star_point = Point1(i,j);
		}
	}
}


/**
* Set edges correponding to a line starting from the border of the image to the star point
**/
inline void draw_line(CImg<bool> &edges_status_out, GraphType &G,
					  int &x, int &y, int &X, int &Y,
					  int &nx, double &beta, bool &auto_background, bool &star_shape_prior,
					  CImg<double> &edges)
{
	if (auto_background)
		G.add_tweights(y*nx+x, INF, 0);

	if (!star_shape_prior)
		return;

	int dx = (x<X) ? 1 : (-1);
	int dy = (y<Y) ? 1 : (-1);

	//Current point: (i,j)
	int i,j;

	if (abs(Y-y) > abs(X-x))
	{
		double slopey = 1.*(X-x)/(Y-y);
		//Parcours en y
		//cout << "Parcours en y" << endl;
		for (int j=y; j != Y; j+=dy)
		{
			i = round((j-y)*slopey + x);

			//Next point: ((j+dy-y)*slopey + x, j+dy)
			int next_i = round((j+dy-y)*slopey + x);
			int next_j = j+dy;

			dx = next_i - i;

			G.add_edge(j*nx + i, next_j*nx + next_i, beta, INF);
#ifdef TEST
			//edges.draw_arrow(ZOOM*i, ZOOM*j, ZOOM*next_i, ZOOM*next_j, &col, 20, 10);
			double col = 0;
			edges.draw_line(ZOOM*i, ZOOM*j, ZOOM*next_i, ZOOM*next_j, &col);
#endif

			if (   (!edges_status_out(next_i, next_j)) ||
				   ((next_i == X) && (next_j == Y))   )
			{
				edges_status_out(next_i, next_j) = true;
			}
			else
				return;
		}
	}
	else
	{
		//Parcours en x
		//cout << "Parcours en x" << endl;
		double slopex = 1.*(Y-y)/(X-x);

		for (int i=x; i != X; i+=dx)
		{
			j = round(slopex * (i-x) + y);

			//Next point: (i+dx, slope * (i+dx-x) + y)
			int next_i = i+dx;
			int next_j = round(slopex * (i+dx-x) + y);

			dy = next_j - j;

			G.add_edge(j*nx + i, next_j*nx + next_i, beta, INF);
#ifdef TEST
			double col = 0;
			edges.draw_line(ZOOM*i, ZOOM*j, ZOOM*next_i, ZOOM*next_j, &col);
			//edges.draw_arrow(ZOOM*i, ZOOM*j, ZOOM*next_i, ZOOM*next_j, &col, 20, 10);
#endif
			if (   (!edges_status_out(next_i, next_j)) ||
				   ((next_i == X) && (next_j == Y))   )
			{
				edges_status_out(next_i, next_j) = true;
			}
			else

				return;

		}
	}
	
}



inline void draw_edges_image_data(GraphType &G, CImg<double> &image,
								  int &x1, int &y1, int x2, int y2, int &nx, int &ny,
								  double &lambda, CImg<double> &sigmas, double &sigma_hard, double dist,
								  CImg<double> &edges)
{
	if (x2 >= 0 && x2<nx && y2>=0 && y2 < ny)
	{
		double sigma;
		if (sigma_hard != 0)
			sigma = sigma_hard;
		else
			sigma = sigmas(x1, y1);

		double weight = lambda * exp(-diff(image, x1, y1, x2, y2)/(2*sigma*sigma)) / dist;
		if (sigma == 0)
			weight = 0;

		G.add_edge(y1*nx+x1, y2*nx+x2, weight, weight);	//Voisin du haut
		double col = 0;
#ifdef TEST
		edges.draw_line(ZOOM*x1, ZOOM*y1, ZOOM*x2, ZOOM*y2, &col);
#endif
	}
}



int gc_main_old( )
{
	CImg<double> image;
	CImg<double> mask;

	//Algo parameters
	double lambda;
	double sigma = 0;
	double beta = 0;
	double force_type = 0;
	double force;

	bool auto_background;
	bool star_shape_prior;
	bool compute_beta = false;

#ifndef TEST
    /*
	lambda = atof(argv[1]);
	sigma  = atof(argv[2]);
	
	auto_background  = (atof(argv[3]) == 1);
	star_shape_prior = (atof(argv[4]) == 1);
	
	force_type = atof(argv[5]);
	force = atof(argv[6]);
    */
	lambda = 20;
	sigma  = 0;
	
	auto_background  = 0;
	star_shape_prior = 0;
	
	force_type = BOUNDARY_BALLOONING;
	force = -10.0;

	if (force_type == BOUNDARY_BALLOONING)
	{
		beta = force;

		if (force == -234.12)
			compute_beta = true;
	}

	cout << "Force type=" << force_type << endl;
	cout << "Lambda=" << lambda << endl;
	cout << "Sigma=" << sigma << endl;
	if (!compute_beta)
		cout << "Beta=" << beta << endl;
	else
		cout << "Optimal beta search activated" << endl;
	cout << "Auto-Background=" << auto_background << endl;
	cout << "Star Shape=" << star_shape_prior << endl;
#else
	cout << "TEST " << endl << endl;
	//Parameters
	lambda = 10;
	beta = -6;
	auto_background = true;
	star_shape_prior = true;

#endif

	//Loads images
	image.load("image.bmp");
	//image = image.RGBtoLab();
	mask.load("mask.bmp");
	mask = mask.get_channel(0);

	unsigned int w = image.width;
	unsigned int h = image.height;

	CImg<double> sigmas(w,h);
	cout << "compute sigma..."<<endl;
	if (sigma == 0)
		compute_sigmas(image, sigmas);
    else
       sigmas.load("sigmas.bmp");
	cout << "sigmas is done "<<endl;

#ifdef TEST
	//display_image(sigmas);
#endif
	//sigmas.save("sigmas.bmp");

	//Loads object.background seeds
	std::vector<Point1> object_points;
	std::vector<Point1> background_points;
	Point1 star_point(-1,-1);
    // mask(402,141) = 2;
	find_points(mask, object_points, background_points, star_point);
	cout << "Star point is: " << star_point.x << " " << star_point.y << endl;
	cout << object_points.size() << " object points." << endl;
	cout << background_points.size() << " background points." << endl;

	if (star_point.x == -1)
	{
		cout << "No star point provided..." << endl;
		system("pause");
		return 1;
	}

	int X = star_point.x;
	int Y = star_point.y;

#ifdef TEST
	//Display points coordinates
	cout << "Object points:" << endl;
	for (unsigned int i=0; i<object_points.size(); ++i)
		cout << object_points[i].x << " " << object_points[i].y << endl;
	cout << endl;

	cout << "Background points:" << endl;
	for (unsigned int i=0; i<background_points.size(); ++i)
		cout << background_points[i].x << " " << background_points[i].y << endl;
	cout << endl;

#endif




	CImg<double> edges(w*ZOOM-(ZOOM-1), h*ZOOM-(ZOOM-1), 1, 1, 255);




	//Finding optimal beta
	double beta_min = -30;
	double beta_max = 0;
	int iters_max = 10;

	double beta_sup = beta_max;
	double beta_inf = beta_min;
	if (compute_beta)
		beta = (beta_sup + beta_inf)/2;

	int nb_iters = 0;
	int nx=w, ny=h;	// Sans les bords
	GraphType G(nx*ny,32*nx*ny);

	while (true)
	{
		cout << "Iteration " << nb_iters << endl;
		cout << "Beta=" << beta << endl;

		// Graphe
		cout << "Setting graph...";

		G.reset();
		G.add_node(nx*ny);

		//Data term
		for (int i=0; i<nx; i++)
		{
			for (int j=0; j<ny; j++)
			{
				G.add_tweights(j*nx+i, 0, 0);

				draw_edges_image_data(G, image, i, j, i, j+1, nx, ny, lambda, sigmas, sigma, 1, edges);	//Down
				draw_edges_image_data(G, image, i, j, i+1, j, nx, ny, lambda, sigmas, sigma, 1, edges);	//Right
				draw_edges_image_data(G, image, i, j, i+1, j-1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Top right
				draw_edges_image_data(G, image, i, j, i+1, j+1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Down right

			}
		}

#ifdef TEST
		edges.fill(255);
#endif

		//Seed object points
		for (unsigned int i=0; i<object_points.size(); ++i)
		{
			int x = object_points[i].x;
			int y = object_points[i].y;
			G.add_tweights(y*nx+x, 0, INF);
		}

		//Star point
		G.add_tweights(Y*nx+X, 0, INF);

		//Background object points
		for (unsigned int i=0; i<background_points.size(); ++i)
		{
			int x = background_points[i].x;
			int y = background_points[i].y;
			G.add_tweights(y*nx+x, INF, 0);
		}



		////Star shape edges for each line from the center to any pixel
		//CImg<bool> edges_status_out(w, h, 9, 1, false);
		//for (int x=0; x<w; ++x)
		//{
		//	for (int y=0; y<h; ++y)
		//	{
		//		if ((abs(x-X) + abs(y-Y)) > 2)
		//			draw_line(edges_status_out, G, x, y, X, Y, nx, beta, auto_background, star_shape_prior, edges);
		//	}
		//}


        //Star shape edges for each line from C to the border
        CImg<bool> edges_status_out(w, h, 1, 1, false);
		cimg_for_borderXY(image, x, y, 1)
			draw_line(edges_status_out, G, x, y, X, Y, nx, beta, auto_background, star_shape_prior, edges);

		cout << "Done." << endl;

		if (force_type == UNIFORM_BALLOONING)
		{
			//Ballooning force
			double k = 1;
			for (int x=0; x<w; ++x)
			{
				for (int y=0; y<h; ++y)
				{
					if (!(x==X && y==Y))
					{
						G.add_tweights(y*nx+x, -force, 0);
					}
				}
			}
		}

		if (force_type == FORCE_BALLOONING)
		{
			//Ballooning force
			for (int x=0; x<w; ++x)
			{
				for (int y=0; y<h; ++y)
				{
					if (!(x==X && y==Y))
					{
						double tweight = - force * 1. / sqrt((double) ((x-X)*(x-X) + (y-Y)*(y-Y)) );
						G.add_tweights(y*nx+x, tweight, 0);

						//double force = - k * 1. / sqrt((double) ((x-X)*(x-X) + (y-Y)*(y-Y)) );
						//G.add_tweights(y*nx+x, force, 0);
					}
				}
			}
		}



#ifdef TEST
		//display_image(edges.get_resize(-400, -400), 1000);
		edges.save("star_prior_term.bmp");
#endif

		// Coupe
		cout << "Computing Cut..." << endl;
		double f = G.maxflow();
		cout << f << endl;

		if (!compute_beta)
			break;


		// Segmentation result
		int nb_objects=0;
		for (int j=0;j<ny;j++) {
			for (int i=0;i<nx;i++) {
				if (G.what_segment(j*nx+i)==GraphType::SINK)
					nb_objects++;
			}
		}
		cout << "Object contains " << nb_objects << " points" << endl << endl;

		//New beta
		if (nb_objects < 1000)
		{
			//On baisse le beta
			beta_inf = beta_inf;
			beta_sup = beta;
			beta = (beta_inf + beta)/2;

			if (beta - beta_min < 0.05)
			{
				cout << "Diminution du beta..." << endl;
				beta_inf -= 50;
				beta_min = beta_inf;
				beta_sup = beta;
				beta = (beta_inf + beta)/2;
				nb_iters = 0;
			}
		}
		else
		{
			if (nb_iters >= iters_max)
				break;

			//On augmente le beta
			beta_inf = beta;
			beta_sup = beta_sup;
			beta = (beta_sup + beta)/2;

			if (beta_max - beta < 0.05)
			{
				cout << "augmentation du beta..." << endl;
				beta_sup += 50;
				beta_max = beta_sup;
				beta_inf = beta;
				beta = (beta_sup + beta)/2;
				nb_iters = 0;
			}
		}

		nb_iters++;
	}


	// Dessin de la zone objet
	for (int j=0;j<ny;j++) {
		for (int i=0;i<nx;i++) {
			if (G.what_segment(j*nx+i)==GraphType::SINK)
			{
				//image(i,j,0) = 0;
				image(i,j,1) = image(i,j,1)*0.5 + 255 * 0.5;
				//image(i,j,2) = 0;
			}
		}
	}

	//Dessin object points
	for (unsigned int in=0; in<object_points.size(); ++in)
	{
		for (int i=-2; i<=2; ++i)
		{
			for (int j=-2; j<=2; ++j)
			{
				image(object_points[in].x+i, object_points[in].y+j, 0) = 255;
				image(object_points[in].x+i, object_points[in].y+j, 1) = 255;
				image(object_points[in].x+i, object_points[in].y+j, 2) = 0;
			}
		}
	}

	//Star point
	for (int i=-2; i<=2; ++i)
	{
		for (int j=-2; j<=2; ++j)
		{
			image(X+i, Y+j, 0) = 0;
			image(X+i, Y+j, 1) = 0;
			image(X+i, Y+j, 2) = 255;
		}
	}

	//Dessin background points
	for (unsigned int in=0; in<background_points.size(); ++in)
	{
		for (int i=-2; i<=2; ++i)
		{
			for (int j=-2; j<=2; ++j)
			{
				image(background_points[in].x+i, background_points[in].y+j, 0) = 0;
				image(background_points[in].x+i, background_points[in].y+j, 1) = 255;
				image(background_points[in].x+i, background_points[in].y+j, 2) = 255;
			}
		}
	}



#ifdef TEST
	display_image(image);
	system("pause");
#endif

	image.save("results.bmp");

	return 0;
}


int gc_main()
{

	//Algo parameters
	double lambda;
	double beta = 0;
	double force_type = 0;
	double force;
    double sigma = 0;

	bool auto_background;
	bool star_shape_prior;
	bool compute_beta = false;

	lambda = 20;
	//sigma  = 0;
    //VOIS = sigma;
	
	auto_background  = 0;
	star_shape_prior = 0;
	
	force_type = BOUNDARY_BALLOONING;
	force = -10.0;

	if (force_type == BOUNDARY_BALLOONING)
	{
		beta = force;

		if (force == -234.12)
			compute_beta = true;
	}
    /*
	cout << "Force type=" << force_type << endl;
	cout << "Lambda=" << lambda << endl;
	cout << "Sigma=" << sigma << endl;
	if (!compute_beta)
		cout << "Beta=" << beta << endl;
	else
		cout << "Optimal beta search activated" << endl;
	cout << "Auto-Background=" << auto_background << endl;
	cout << "Star Shape=" << star_shape_prior << endl;
    */


	//Loads images
	//Loads images
	//image.load("image.bmp");
	//image = image.RGBtoLab();
	//mask.load("mask.bmp");
	//mask = mask.get_channel(0);

    BYTE **I;
    Bitmap *orig_img = NULL;
    int h1, w1;
    orig_img = load_image("image.bmp", h1, w1);
    CImg<double> mask(w1,h1,1,1,false);
	CImg<double> image(w1,h1,1,1,false);
    image_height = h1;
    image_width  = w1;
    set_image(I, orig_img);
    cout << "ok" << endl;
    for (int i = 0 ; i < w1; i ++ )
       for (int j = 0; j < h1; j ++) {
          image(i,j,0,0) =  I[j][i] == BLACK ? 0 : 255;
          mask(i,j,0,0) = 255;
       }
    cout << "set" << endl;
    mask = mask.get_channel(0);


	unsigned int w = image.width;
	unsigned int h = image.height;

    /*
    for (int i = 0 ; i < w; i ++ )
       for (int j = 0; j < j; j ++) {
          mask(i,j,0)=255;
          mask(i,j,1)=255;
          mask(i,j,2)=255;
       }
	mask = mask.get_channel(0);
    */

    image_width = w;
    image_height = h;
    BYTE ***im_result_region = new_3D_image(h, w);
    BYTE ***im_result = new_3D_image(h, w);

    mask(402, 141) = 255;
    for (int i = 0 ; i < h; i ++ )
       for (int j = 0; j < w; j ++)
          if ( I[i][j] == BLACK )
          {
             im_result[i][j][0] = 0;
             im_result[i][j][1] = 0;
             im_result[i][j][2] = 0;
          }
    
    int nb_iters = 0;
	while (true)
	{
    
       CImg<double> sigmas(w,h);
       compute_sigmas(image, sigmas);

       // find the star point
       bool found = false;
       for (int i = 0 ; i < w && !found; i ++ ) 
          for (int j = 0; j < h && !found; j ++) 
             if ( I[j][i] == BLACK ) {
                mask(i,j) = 2;
                cout << "Star point: " << i << ", " << j <<endl;
                found = true;
             }


       if (!found) {
          cout << "Not found star point! " <<endl;
          break;
       }
          
       //Loads object.background seeds
       std::vector<Point1> object_points;
       std::vector<Point1> background_points;
       Point1 star_point(-1,-1);

       
       // mask(402,141) = 2;
       find_points(mask, object_points, background_points, star_point);
       cout << "Star point is: " << star_point.x << " " << star_point.y << endl;
       cout << object_points.size() << " object points." << endl;
       cout << background_points.size() << " background points." << endl;

       int X = star_point.x;
       int Y = star_point.y;

       CImg<double> edges(w*ZOOM-(ZOOM-1), h*ZOOM-(ZOOM-1), 1, 1, 255);


       //Finding optimal beta
       double beta_min = -30;
       double beta_max = 0;
       int iters_max = 10;

       double beta_sup = beta_max;
       double beta_inf = beta_min;
       if (compute_beta)
          beta = (beta_sup + beta_inf)/2;

       int nx=w, ny=h;	// Sans les bords
       GraphType G(nx*ny,32*nx*ny);

       // old while starts here.

		cout << "Iteration " << nb_iters << endl;
		cout << "Beta=" << beta << endl;

		// Graphe
		cout << "Setting graph...";

		G.reset();
		G.add_node(nx*ny);

		//Data term
		for (int i=0; i<nx; i++)
		{
			for (int j=0; j<ny; j++)
			{
				G.add_tweights(j*nx+i, 0, 0);

				draw_edges_image_data(G, image, i, j, i, j+1, nx, ny, lambda, sigmas, sigma, 1, edges);	//Down
				draw_edges_image_data(G, image, i, j, i+1, j, nx, ny, lambda, sigmas, sigma, 1, edges);	//Right
				draw_edges_image_data(G, image, i, j, i+1, j-1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Top right
				draw_edges_image_data(G, image, i, j, i+1, j+1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Down right

			}
		}

		//Seed object points
		for (unsigned int i=0; i<object_points.size(); ++i)
		{
			int x = object_points[i].x;
			int y = object_points[i].y;
			G.add_tweights(y*nx+x, 0, INF);
		}

		//Star point
		G.add_tweights(Y*nx+X, 0, INF);

		//Background object points
		for (unsigned int i=0; i<background_points.size(); ++i)
		{
			int x = background_points[i].x;
			int y = background_points[i].y;
			G.add_tweights(y*nx+x, INF, 0);
		}



        //Star shape edges for each line from C to the border
        CImg<bool> edges_status_out(w, h, 1, 1, false);
		cimg_for_borderXY(image, x, y, 1)
			draw_line(edges_status_out, G, x, y, X, Y, nx, beta, auto_background, star_shape_prior, edges);

		cout << "Done." << endl;

		// Coupe
		cout << "Computing Cut..." << endl;
		double f = G.maxflow();
		cout << f << endl;


		nb_iters++;

        int r = rand()%255;
        int g = rand()%255;
        int b = rand()%255;
        int count = 0;
        for (int j=0;j<ny;j++) {
           for (int i=0;i<nx;i++) {
              if (G.what_segment(j*nx+i)==GraphType::SINK)
              {
                 im_result_region[j][i][0] = r;
                 im_result_region[j][i][1] = g;
                 im_result_region[j][i][2] = b;
                 im_result[j][i][0] = !im_result[j][i][0] ? r : im_result[j][i][0];
                 im_result[j][i][1] = !im_result[j][i][1] ? g : im_result[j][i][1];
                 im_result[j][i][2] = !im_result[j][i][2] ? b : im_result[j][i][2];
                 image(i,j,0) = 255;
                 I[j][i] = WHITE;
                 count ++;
              }
           }
        }
        cout << "INFO: # of region is " << count << endl;

        mask(X, Y) = 0;
        
	}

    Save2File("results_seg.png", NULL, NULL, image_width, image_height, 1, im_result);
    Save2File("results_seg_region.png", NULL, NULL, image_width, image_height, 1, im_result_region);
    free_image_3D(im_result);
    free_image_3D(im_result_region);

    return 0;
}

st_table *comp_seg_gc_for_region(BYTE **I,
                                 int p_index,
                                 int threshold,
                                 st_table *st_new_data,
                                 st_table *pixel2rid)
{

   // construct the st_table
   st_table *st_region = st_init_table(st_ptrcmp, st_ptrhash);
   st_insert(st_new_data, (char *)p_index, (char *) 0);

   // graph-cut

	//Algo parameters
	double lambda;
	double beta = 0;
	double force_type = 0;
	double force;
    double sigma = 0;

	bool auto_background;
	bool star_shape_prior;
	bool compute_beta = false;
	lambda = 20;
    VOIS = threshold;
	
	auto_background  = 0;
	star_shape_prior = 0;
	
	force_type = BOUNDARY_BALLOONING;
	force = -10.0;

	if (force_type == BOUNDARY_BALLOONING)
	{
		beta = force;

		if (force == -234.12)
			compute_beta = true;
	}

	//Loads images

	unsigned int w = image_width;
	unsigned int h = image_height;

	CImg<double> image(w,h,1,1,false);
    CImg<double>  mask(w,h,1,1,false);
    for (int i = 0 ; i < w; i ++ )
       for (int j = 0; j < h; j ++) {
          image(i,j,0,0) =  I[j][i] == BLACK ? 0 : 255;
          mask(i,j,0,0)  =  255;
       }
    mask = mask.get_channel(0);

    I[0][p_index] = WHITE;
    
	//if ( 1 )
	{
    
       CImg<double> sigmas(w,h);
       compute_sigmas(image, sigmas);

       // find the star point
       int start_x = x_axis(p_index);
       int start_y = y_axis(p_index);
       mask(start_x, start_y) = 2;
       cout << "Star point: " << start_x << ", " << start_y << " VOIS: " << VOIS << endl;

          
       //Loads object.background seeds
       std::vector<Point1> object_points;
       std::vector<Point1> background_points;
       Point1 star_point(-1,-1);

       
       find_points(mask, object_points, background_points, star_point);
       //star_point = Point1(start_x, start_y);
       cout << "Star point is: " << star_point.x << " " << star_point.y << endl;
       cout << object_points.size() << " object points." << endl;
       cout << background_points.size() << " background points." << endl;

       int X = star_point.x;
       int Y = star_point.y;

       CImg<double> edges(w*ZOOM-(ZOOM-1), h*ZOOM-(ZOOM-1), 1, 1, 255);


       //Finding optimal beta
       double beta_min = -30;
       double beta_max = 0;

       double beta_sup = beta_max;
       double beta_inf = beta_min;
       if (compute_beta)
          beta = (beta_sup + beta_inf)/2;

       int nx=w, ny=h;	// Sans les bords
       GraphType G(nx*ny,32*nx*ny);

		// Graphe
		cout << "Setting graph...";

		G.reset();
		G.add_node(nx*ny);

		//Data term
		for (int i=0; i<nx; i++)
		{
			for (int j=0; j<ny; j++)
			{
				G.add_tweights(j*nx+i, 0, 0);

				draw_edges_image_data(G, image, i, j, i, j+1, nx, ny, lambda, sigmas, sigma, 1, edges);	//Down
				draw_edges_image_data(G, image, i, j, i+1, j, nx, ny, lambda, sigmas, sigma, 1, edges);	//Right
				draw_edges_image_data(G, image, i, j, i+1, j-1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Top right
				draw_edges_image_data(G, image, i, j, i+1, j+1, nx, ny, lambda, sigmas, sigma, sqrt(2.), edges);	//Down right

			}
		}

		//Seed object points
		for (unsigned int i=0; i<object_points.size(); ++i)
		{
			int x = object_points[i].x;
			int y = object_points[i].y;
			G.add_tweights(y*nx+x, 0, INF);
		}

		//Star point
		G.add_tweights(Y*nx+X, 0, INF);

		//Background object points
		for (unsigned int i=0; i<background_points.size(); ++i)
		{
			int x = background_points[i].x;
			int y = background_points[i].y;
			G.add_tweights(y*nx+x, INF, 0);
		}



        //Star shape edges for each line from C to the border
        CImg<bool> edges_status_out(w, h, 1, 1, false);
		cimg_for_borderXY(image, x, y, 1)
			draw_line(edges_status_out, G, x, y, X, Y, nx, beta, auto_background, star_shape_prior, edges);

		cout << "Done." << endl;

		// Coupe
		cout << "Computing Cut..." << endl;
		double f = G.maxflow();
		cout << f << endl;

        for (int j=0;j<ny;j++) {
           for (int i=0;i<nx;i++) {
              if (G.what_segment(j*nx+i)==GraphType::SINK)
              {
                  int new_p = index(i, j);
                  
                  if ( !st_is_member(st_region, (char *)new_p ) ) {
                     st_insert(st_region, (char *)new_p, (char *) 0);
                     st_insert(pixel2rid, (char *)new_p, (char *)st_region);
                  }
                  if ( I[j][i] == BLACK ) {
                     I[j][i] = WHITE;
                     st_insert(st_new_data, (char *)new_p, (char *) st_region);
                  }
              }
           }
        }
	}

   // end of graph-cut
    
   return st_region;
}

////// Graph Cut
#endif

typedef struct _ksd_region {
   int rid;
   int start;
   int end;
   int centroid;
   vector<int> data_region;
   vector<int> data_structure;
   unsigned char color[3];
} KSD_REGION;

extern void set_color(BYTE ***im, int y, int x, BYTE r, BYTE g, BYTE b);
extern BYTE *** new_3D_image(int h, int w);
void hash_2_vector(st_table *st, vector<int> &data);

int comp_seg_starting_point(BYTE **I)
{
   // algorithm 1: find the first data as the starting point
   int len = image_width * image_height;
   for (int i = 0; i < len; i++ ) {
      if ( I[0][i] == BLACK )
         return i;
   }

   return -1;
      
}

st_table *comp_seg_dilate_for_region(BYTE **I,
                                     int p_index,
                                      int threshold,
                                     st_table *st_new_data,
                                     st_table *pixel2rid)
{

   // construct the st_table
   st_table *st_region = st_init_table(st_ptrcmp, st_ptrhash);
   st_table *st_new_points = st_init_table(st_ptrcmp, st_ptrhash);
   st_table *st_start_points = st_init_table(st_ptrcmp, st_ptrhash);
   st_insert(st_start_points, (char *)p_index, (char *) 0);
   st_insert(st_new_data, (char *)p_index, (char *) 0);
   I[0][p_index] = WHITE;
   
   while ( st_count(st_start_points) > 0 )
   {
      int *st_index;
      st_generator *stGen;
      st_foreach_item( st_start_points, stGen, (char **)(&st_index), NULL) {
         
         int x0 = x_axis((int)st_index);
         int y0 = y_axis((int)st_index);
         int x, y;

         for ( y = y0 - threshold; y <= y0 + threshold; y ++ )
            for ( x =  x0 - threshold; x <= x0 + threshold; x ++ )
               if ( x > 0 && y > 0 && x < image_width && y < image_height ) {
                  int new_p = index(x, y);
                  if ( !st_is_member(st_region, (char *)new_p ) ) {
                     st_insert(st_region, (char *)new_p, (char *) 0);
                     st_insert(pixel2rid, (char *)new_p, (char *)st_region);
                  }
                  if ( I[y][x] == BLACK ) {
                     I[y][x] = WHITE;
                     st_insert(st_new_points, ( char *)new_p, (char *)0);
                     st_insert(st_new_data, (char *)new_p, (char *) st_region);
                  }
               }
      }
      st_free_table(st_start_points);
      st_start_points = st_new_points;
      st_new_points = st_init_table(st_numcmp, st_numhash);
   }

   return st_region;
}

void draw_3D_region_from_array(char * fn, array_t *all_regions)
{
   BYTE ***im = new_3D_image(image_height, image_width);
   int color_switch = 0;
   int r, g, b;

   int k;
   KSD_REGION *region;
   arrayForEachItem(KSD_REGION*, all_regions, k, region) {
      assert(region);
      r = region->color[0];
      g = region->color[1];
      b = region->color[2];

      for (int i = 0; i < region->data_region.size(); i ++ ) {
         int p_index = region->data_region[i];
         int x = x_axis((int)p_index);
         int y = y_axis((int)p_index);
         set_color(im, y, x, r, g, b);
      }

   }

   arrayForEachItem(KSD_REGION*, all_regions, k, region) {
      assert(region);
      for (int i = 0; i < region->data_structure.size(); i ++ ) {
         int p_index = region->data_structure[i];
         int x = x_axis((int)p_index);
         int y = y_axis((int)p_index);
         set_color(im, y, x, 255, 255, 255);
      }
   }
      
   Save2File(fn, NULL, NULL, image_width, image_height, 1, im);
   free_image_3D(im);
   
}

void draw_3D_region_from_hash(char * fn, st_table *st_all_regions, st_table *st_all_data)
{
   BYTE ***im = new_3D_image(image_height, image_width);
   int color_switch = 0;
   int r, g, b;

   st_generator *sgOut;
   st_generator *sgInner;
   st_table *st_region;
   st_table *st_data;
   st_foreach_item( st_all_regions, sgOut, (char **)&st_region, NULL ) {
      assert(st_region);
      int *p_index;
      if (0) {
         if ( color_switch % 3 == 0 ) {
            r = 255; g = 0; b = 0;
         }
         else if ( color_switch % 3 == 1 ) {
            r = 0; g = 255; b = 0;
         }
         else if ( color_switch % 3 == 2 ) {
            r = 0; g = 0; b = 255;
         }
         color_switch ++;
      } else {
         r = rand() % 255;
         g = rand() % 255;
         b = rand() % 255;
      }
      
      st_foreach_item( st_region, sgInner, (char **)&p_index, NULL ) {
         int x = x_axis((int)p_index);
         int y = y_axis((int)p_index);
         set_color(im, y, x, r, g, b);
      }

      //st_free_table(st_region);
   }

   r=255; g=255; b=255;
   st_foreach_item( st_all_data, sgOut, (char **)&st_data, NULL ) {
      assert(st_data);
      int *p_index;
      st_foreach_item( st_data, sgInner, (char **)&p_index, NULL ) {
         int x = x_axis((int)p_index);
         int y = y_axis((int)p_index);
         set_color(im, y, x, r, g, b);
      }
      //st_free_table(st_data);
   }

   Save2File(fn, NULL, NULL, image_width, image_height, 1, im);
   free_image_3D(im);
   
}

static void comp_segments(char *fn, st_table *&st_all_regions, st_table *&st_all_data, BYTE **&cur_im, st_table *pixel2rid, int thres_mask, int approach = 0)
{
   int threshold = thres_mask; /* How accurate should we segment the objects? */
   
   // test whether the file exist or not
   if ( !file_exist(fn) ) {
      printf("Error: the file %s is not exist!\n", fn);
      exit(0);
   }

   // s1: load the image
#ifdef LINUX
   CImg<BYTE> *orig_img = NULL;
#else
   Bitmap *orig_img = NULL;
#endif
   int h, w;
   orig_img = load_image(fn, h, w);
   image_height = h;
   image_width  = w;
   set_image(cur_im, orig_img);

   BYTE **I = copy_image(cur_im, h, w);

   st_all_regions = st_init_table(st_ptrcmp, st_ptrhash);
   st_all_data = st_init_table(st_ptrcmp, st_ptrhash);

   while ( true ) {
      
      // s2: pick up a starting point $p$ from image I
      int p_index = comp_seg_starting_point(I);
      if ( p_index == -1 )
         break;
      
      // s3: dilating $p$ with mask $m=threshold$, collect new points set $s$.
      // s4: if $s$ is not empty, repeat s3 on points in $s$.
      //     if $s$ is empty, update $I'$ containing dilated pixels, set value 0.
      //                      update $I = I && I'$;
      st_table *st_new_data   = st_init_table(st_ptrcmp, st_ptrhash);
      st_table *st_new_region;

      if ( approach == 0 )  // graph cut based
         st_new_region = comp_seg_dilate_for_region(I, p_index, threshold, st_new_data, pixel2rid);
#ifndef LINUX      
      else
         st_new_region = comp_seg_gc_for_region(I, p_index, threshold, st_new_data, pixel2rid);
#endif      
      
      st_insert(st_all_regions, (char *)st_new_region, (char *) st_new_data);
      st_insert(st_all_data, (char *)st_new_data, (char *) st_new_region);

      // s5: if $I$ is empty, done, otherwise, go to s2.
      // s2 will check p_index
   }

   // free resources;
   free_image(I);
   delete orig_img;

}

KSD_REGION *new_ksd_region(int start)
{
   static int global_unique_region_id = 0;

   KSD_REGION *kr = new KSD_REGION(); // use new (not malloc): there is a class (vector) in KSD_REGION
   kr->rid = global_unique_region_id;
   kr->start = start;
   kr->end = -1; // we dont know yet.
   kr->centroid = 0; //init to zero first
   kr->color[0] = rand() % 256;
   kr->color[1] = rand() % 256;
   kr->color[2] = rand() % 256;
   global_unique_region_id ++;

   return kr;
}

BYTE **load_image_to_array(char *fn)
{
#ifdef LINUX
   CImg<BYTE> *orig_img = NULL;
#else
   Bitmap *orig_img = NULL;
#endif
   int h, w;
   BYTE **cur_im;
   orig_img = load_image(fn, h, w);
   set_image(cur_im, orig_img);
   delete orig_img;

   return cur_im;
}

static bool find_matched_region (KSD_REGION *region, st_table *st_all_regions, vector<int> &new_region, st_table *&st_new_region, st_table *pixel2rid, float threshold)
{
   st_table *st_rid_count = st_init_table(st_ptrcmp, st_ptrhash);
   int max_matched = 1;
   st_table *max_matched_rid = NULL;
   bool is_matched = false;
   
   // go through each pixel in region
   for (int i = 0; i < region->data_region.size(); i ++ )
   {
      st_table *rid;
      int ret = st_lookup(pixel2rid, (char *)(region->data_region[i]), (char **)&rid);
      if ( ret == 0 )
         continue;

      int count;
      if ( !st_lookup(st_rid_count, (char *)rid, (char **)&count) ) {
         count = 1;
      } else {
         count = count + 1;
         if ( count > max_matched ) {
            max_matched = count;
            max_matched_rid = rid;
         }
      }
      st_insert(st_rid_count, (char *)rid, (char *)count);
   }

   st_free_table(st_rid_count);
   
   // premilinary check on size of the region
   int size_region = region->data_region.size();
   int size_matched = max_matched_rid != NULL ? st_count(max_matched_rid) : 0;
   if ( max_matched_rid == NULL || abs( size_region - size_matched )/(float)max(size_region, size_matched) > 1-threshold )
      return false;
   
   // printf ("FIND_MATCHED_REGION: region total: %d, matched: %d.\n", region->data_region.size(), max_matched);

   st_new_region = max_matched_rid;
   if ( float(max_matched) / float(region->data_region.size()) >= threshold) {
      hash_2_vector(max_matched_rid, new_region);
      is_matched = true;
   } else {
      is_matched = false;
   }

   return is_matched;
}

static float seg_matched_ratio ( vector<int> &region, BYTE **im)
{
   int total_num = region.size();
   int count = 0;

   for (int i = 0; i < total_num; i ++ ) {
      if ( im[0][region[i]] == BLACK )
         count ++;
   }

   return (float)count/(float)total_num;
}

static bool find_matched_structure(vector<int> &region, vector<int> &new_region, BYTE **ref_im, BYTE **new_im, float threshold)
{
   float ref_on_new = 0.0;
   float new_on_ref = 0.0;

   ref_on_new = seg_matched_ratio(region, new_im);
   new_on_ref = seg_matched_ratio(new_region, ref_im);

   if ( max(ref_on_new, new_on_ref) > threshold )
      return true;
   else
      return false;
      
}

void hash_2_vector(st_table *st, vector<int> &data)
{
   st_generator *stGen;
   int *point;
   st_foreach_item(st, stGen, (char **)&point, (char **)NULL) {
      data.push_back((int)point);
   }
}


void vector_region_to_image(BYTE **&ref_im, vector<int> &data_structure)
{
   ref_im = new_image(image_height, image_width);
   for (int i = 0; i < data_structure.size(); i ++ ){
      ref_im[0][data_structure[i]] = BLACK;
   }
}

void array_region_to_image(BYTE **&ref_im, array_t *gbl_active_regions)
{
   ref_im = new_image(image_height, image_width);
   int k;
   KSD_REGION *region;
   arrayForEachItem(KSD_REGION *, gbl_active_regions, k, region) {
      for (int i = 0; i < region->data_structure.size(); i ++ ){
         ref_im[0][region->data_structure[i]] = BLACK;
      }
   }
}

// precondition:
// output_prefix is the output dir
static void ksd_generate_polygon(KSD_REGION *region, char *output_prefix, char *IR_fn_name, bool db)
{
   // generate image;
   BYTE **new_im = new_image(image_height, image_width);
   for (int i = 0; i < region->data_structure.size(); i ++ )
      new_im[0][region->data_structure[i]] = BLACK;

   // do the closing/thinning for points
   {
      int thres = 10;
      for (int i = 0; i < thres; i ++)
         image_dilation(new_im);
      for (int i = 0; i < thres; i ++)
         thinningImage(new_im, image_height, image_width);
   }

   
   // save the region image
   char fn[1000];
   sprintf(fn, "%s/region_%d.tif", output_prefix, region->rid);
   Save2File(fn, new_im, NULL, image_width, image_height, 0, NULL, 1); // 1: tif

   // boundary computation
   {
      char command[1000];
      sprintf(command, "..\\ras2vec.exe -p -o %s %s > log", output_prefix, fn);
      char *command_slash = str_replace(command, "/", "\\", 0, -1);
      printf("Command: %s\n", command_slash);
      system(command_slash);
   }

   // convert the .ply file to IR format
   if ( IR_fn_name )
   {
      sprintf(fn, "%s/region_%d.ply", output_prefix, region->rid);

      if ( !file_exist(fn) ) {
         printf("WARNING: ras2vec.exe on %s is failed\n", fn);
         return;
      }
      
      char line[1000];
      char IR_fn[1000];
      sprintf(IR_fn, "%s/%s", output_prefix, IR_fn_name);
      
      FILE *fd = fopen(fn, "r");
      FILE *fd_out = fopen(IR_fn, "at");
      assert(fd);
      assert(fd_out);
      
      while ( fgets(line, 1000, fd )) {
         if ( strncmp(line, "POLY", 4) == 0) {
            fprintf(fd_out, "BEGIN POLYGON\n");
         }
         else if ( strncmp(line, "END", 3) == 0 ) {
            fprintf(fd_out, "EXTRUSION\n");
            fprintf(fd_out, "HEIGHT\n");
            fprintf(fd_out, "%f %f\n", (float)region->start/gbl_opts->total_slices, (float)region->end/gbl_opts->total_slices);
            fprintf(fd_out, "END POLYGON\n");
         } else {
            fprintf(fd_out, line);
         }
      }
      fclose(fd);
      fclose(fd_out);
      
   }

   free_image(new_im);

   // clean files
   if ( !db )
   {
      char fn1[1000];
      char fn2[1000];
      sprintf(fn1, "%s/region_%d.ply", output_prefix, region->rid);
      sprintf(fn2, "%s/region_%d.tif", output_prefix, region->rid);
      char command[1000];
      char *del_cmd = NULL;
#ifdef LINUX
      del_cmd = "rm";
#else
      del_cmd = "del";
#endif      
      sprintf(command, "%s %s %s", del_cmd, fn1, fn2);
      char *command_slash = str_replace(command, "/", "\\", 0, -1);
      system(command_slash);
   }

}

void draw_2D_region_for_hash(char * fn, st_table *st_all_regions)
{
   BYTE **im = new_image(image_height, image_width);

   st_generator *stGen;
   int *p_index;
   st_foreach_item(st_all_regions, stGen, (char **)&p_index, (char **)NULL) {
      im[0][(int)p_index] = BLACK;
   }

   Save2File(fn, im, NULL, image_width, image_height, 0, NULL);

   free_image(im);
}

void draw_3D_region_for_each_layer(char * fn, st_table *st_all_regions)
{
   BYTE ***im = new_3D_image(image_height, image_width);
   int color_switch = 0;
   int r, g, b;

   printf("Drawing image %s\n", fn);
   st_generator *sgOut;
   st_generator *sgInner;
   st_table *st_data;
   int *slice;
   char *col_str;
   st_foreach_item( st_all_regions, sgOut, (char **)&slice, (char **)&st_data ) {
      int *p_index;
      if (1) {
         if ( color_switch % 3 == 0 ) {
            r = 255; g = 0; b = 0;
            col_str = "red";
         }
         else if ( color_switch % 3 == 1 ) {
            r = 0; g = 255; b = 0;
            col_str = "green";
         }
         else if ( color_switch % 3 == 2 ) {
            r = 0; g = 0; b = 255;
            col_str = "blue";
         }
         color_switch ++;
      } else {
         r = rand() % 255;
         g = rand() % 255;
         b = rand() % 255;
      }

      printf("Draw layer %d with color %s, total %d points\n", (int)slice, col_str, st_count(st_data));
      st_foreach_item( st_data, sgInner, (char **)&p_index, NULL ) {
         int x = x_axis((int)p_index);
         int y = y_axis((int)p_index);
         set_color(im, y, x, r, g, b);
      }

      //st_free_table(st_region);
   }

   Save2File(fn, NULL, NULL, image_width, image_height, 1, im);
   free_image_3D(im);
   
}

void ksd_array_to_region(st_table *st_end_pts, int end_slice, char *output_prefix, char *IR_fn)
{
   // first let's check the biggest region
   {
      // let's order the table based on # of the pts
      // vector<st_table *> arr_tbl;
      // vector<int> arr_index;
      st_generator *stGen;
      int *start_slice;
      st_table *st_pts;
      st_foreach_item( st_end_pts, stGen, (char **)&start_slice, (char **)&st_pts) {
         /*
         {
            for (int i = 0; i < arr_tbl.size(); i ++ ) {
               if ( st_count(st_pts) > st_count(arr_tbl[i]) ) {
                  arr_tbl.insert(arr_tbl.begin() + i, st_pts);
                  arr_index.insert(arr_index.begin() + i, (int)start_slice);
                  isInserted = true;
                  break;
               }
            }

            if ( ! isInserted ) {
               arr_tbl.push_back(st_pts);
               arr_index.push_back((int)start_slice);
            }
         }
         */

         /*
         if ( st_count( st_pts ) < 4 )
            continue;
         */
         
         KSD_REGION *region = new_ksd_region((int)start_slice);
         region->end = end_slice;
         hash_2_vector(st_pts, region->data_structure);

         // vectorize the region data
         ksd_generate_polygon(region, output_prefix, IR_fn, false);

         delete region;
      }
      
      // assert( arr_tbl.size() == arr_index.size());
   }
}

/* 
Function: ksd_with_segmentation

Command:
     Project.exe -K -O "-L 467 -R 470 -f seg_ir.txt -A 1 -s 1000 -t 0.2
     -P result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_recovered/image_slice
     -O result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_seg_result_1000_1_1"

Command to generate iv model:
     project -j -O "-p result\point_cloud_interior_GCT_1000\seg_ir.txt
     -P result\point_cloud_interior_GCT_1000\seg_ir.iv
     -t 11 -0 1 -1 0 -D 0 -M 2 -w 640 -W 1024 -x -15.36 -X 80.23 -y -30.18 -Y 89.93 -z -43.69 -Z 5.69"
     
Parameters:
     passing through a 2-D void pointer.

Data structures:
     Region: id, vector data array; color array.
     Table T: array of regions
     Map M1, M2: st hash_table of ptr
     Log_table: region id -> <start, end>;
     
Algorithms:

     step 1: compute local segmentation based on Graph Cut
     
             a threshold is used to control the granuarity of the segmentation.
             an active table, T, is maintained to store the active regions in current
             reference image. For each new image I, a group of new regions are computed.
             these regions are stored in map M1 ( pixel -> region ) and map M2 ( region -> 0 ).
             for each region, a random triple representing the color of this region is
             assigned to the structure for debug purpose.
   
     step 2: key regions computation and boundary vectorization
   
             for each region in T of reference image, identify the corresponding region
             detected by the new image I by looking up the map M1. If there is one found,
             compute the similarity between these two regions. If they are deemed as same,
             mark this in both T (active/no update) and M2 (regions -> 1). If not (either
             they are deemed as two different keyslices or no corresponding region is found), move
             this region out of T (by creating a new table T'), and update the information in 
             the final log file L (region id, start no, end no). And call BPA process to
             generate polygons for this region, the file name can be names as Rid.txt
             go through each region in M2, if it is not marked with 1 (a new region), add
             it to the active table (T'). Finally, assign T' to T.
             for debug purpose, a color image is saved whenever a status is changed - new
             region was added or old region ended.
   
     step 3: final IR generation based on log file

             for each entry in the final log file L, Assert the  file Rid.txt exists.
             integrate all polygons based on Rid.txt, start and end number of the slices.
     
*/
#if 1
void ksd_with_segmentation(void **params)
{

   // initialize the global vars
   image_width = -1;
   image_height = -1;
   
   // some thresholds for comparison and matching
   /*
     Here we have 3 thresholds for segementation and keyslices detection.
     To merger all these 3 threshold, the algorithm is based on the observation:
     1 - 0.9 -0.8; 5 - 0.5 - 0.4; 9 - 0.1 - 0.1
     We can choose a unique threshold \tau_d to be [0, 1].
     thres_mask would be (int)(10*\tau_d); => [0, 10];
     thres_region = thres_data = (1 - \tau_d) => [1, 0];  // what does 0 matched mean :P
     we should have a way to set these 3 parameters separately.
    */
   int   thres_mask   = 3;   /* mask for region */
   float thres_region = 0.5; /* matched region ratio */
   float thres_match  = 3;   /* matched data ratio */
   //const float thres_region = 0.9; /* matched region ratio */
   //const float thres_data   = 0.8; /* matched data ratio */
   
   // decode the parameters
   bool db = true; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_prefix  = (char *) params[3];
   char *IR_fn = (char *) params[4];
   thres_mask   = (int) params[5];
   thres_region = *((float *) params[6]);
   thres_match   = *((float *) params[7]);
   float tau_d  = *((float *) params[8]);
   int  seg_app = (int) params[9];

   /*
     Here the idea is to dynamically track the changing/unchanging region of the data
     Thinning will help to fill the gap between data, which should be proceeded by dilation.
     For the input, we may/maynot do the closing for the data with threshold N.
     For the comparison, we may/maynot do the closing for the new input with threshold M.
     For each data point, we will see whether it has a close match in new image(closing) within K neighborhood.
     If yes, the data point remains in the map. Otherwise, move it to the set of end region found.
     For each new data point, if it was found to be close to a old data point, mark it and throw it away.
     Otherwise, mark it as a new region.
     For each data point, maintaining a structure storing the information, such as in which slice to start.
    */

   assert( input_prefix );
   assert( output_prefix );

   // clean file if exist
   char clean_IR_fn[1000];
   sprintf(clean_IR_fn, "%s/%s", output_prefix, IR_fn);
   FILE *fd_out = fopen(clean_IR_fn, "w");
   assert(fd_out);
   fclose(fd_out);

   char *fn_ext = NULL;
   {  // let's guess the image extension file.
      char fn[1000];
      sprintf(fn, "%s_0100.tif", input_prefix);
      if ( !file_exist(fn) ) {
         fn_ext = strdup("png");
      } else {
         fn_ext = strdup("tif");
      }
   }
   
   // start the computation
   int k;
   char fn[1000];
   st_generator *stGen;
   st_table *st_active_data = NULL;
   for (int slice_index = left_num; slice_index < right_num; slice_index += 3 )
   {
      char *num_str = digit_string(4, slice_index);
      sprintf(fn, "%s_%s.%s", input_prefix, num_str, fn_ext);

      printf("INFO: computing %s\n", fn);

      BYTE **im_thin = NULL;
      BYTE **im_orig = NULL;

      // load image and  do the closing
      {

#ifdef LINUX
         CImg<BYTE> *orig_img = NULL;
#else
         Bitmap *orig_img = NULL;
#endif
         int h, w;
         orig_img = load_image(fn, h, w);
         set_image(im_orig, orig_img);

         im_thin = copy_image(im_orig, h, w);
         int thres = thres_mask;
         for (int i = 0; i < thres; i ++)
            image_dilation(im_thin);
         for (int i = 0; i < thres; i ++)
            thinningImage(im_thin, h, w);

         delete orig_img;

      }

      int res = image_width * image_height;
      st_table *st_thin_data = st_init_table(st_numcmp, st_numhash);
      for (int i = 0; i < res; i ++ )
         if ( im_thin[0][i] == BLACK )
            st_insert(st_thin_data, (char *)i, (char *)slice_index);
      
      if ( db && st_count(st_thin_data) > 0 ) {
         sprintf(fn, "%s/debug/image_of_thinned_%s.png", output_prefix, num_str);
         draw_2D_region_for_hash(fn, st_thin_data);
      }

      // check whether this is a blank image.
      if ( st_count(st_thin_data) == 0 ) {
         free_image(im_thin);
         free_image(im_orig);
         continue;
      }

      if ( slice_index == left_num || st_count(st_active_data) == 0 ) {

         // init the active table T.
         printf("Init the reference due to 0 size of gbl_active_regions\n");
         st_active_data = st_thin_data;
         continue;
      }

      // point-based comparison.
      int *start_slice;
      int *pt;
      st_table *st_matched_pts = st_init_table(st_numcmp, st_numhash);
      st_foreach_item(st_active_data, stGen, (char **)&pt, (char **)&start_slice) {

         // find the matched region
         {
            int x0 = x_axis((int)pt);
            int y0 = y_axis((int)pt);
            for (int x = x0-thres_match; x <= x0+thres_match; x ++ )
               for (int y = y0-thres_match; y <= y0+thres_match; y ++ )
                  if ( valid_pixel(x, y) ) {
                     if ( im_thin[y][x] == BLACK ) {
                        int new_pt = index(x, y);
                        if ( !st_is_member(st_matched_pts, (char *)((int)pt)) ) 
                           st_insert(st_matched_pts, (char *)(pt), (char *)(start_slice));
                        if ( !st_is_member(st_matched_pts, (char *)new_pt) )
                           st_insert(st_matched_pts, (char *)new_pt, (char *)1);
                     }
                  }
         }

      }

      st_table *st_new_pts = st_init_table(st_numcmp, st_numhash);
      st_table *st_end_pts = st_init_table(st_numcmp, st_numhash);
      st_foreach_item(st_active_data, stGen, (char **)&pt, (char **)&start_slice) {

         if ( st_is_member(st_matched_pts, ( char *)(pt)) ) {
            st_insert(st_new_pts, (char *)(pt), (char *)(start_slice) );
         } else {
            st_insert(st_end_pts, (char *)(pt), (char *)(start_slice) );
         }
      }
      st_foreach_item(st_thin_data, stGen, (char **)&pt, (char **)&start_slice) {

         if ( !st_is_member(st_matched_pts, ( char *)(pt)) ) {
            st_insert(st_new_pts, (char *)(pt), (char *)(start_slice) );
         }
      }
      st_free_table(st_active_data);
      st_active_data = st_new_pts;

      // show st_end_pts here;
      if ( db && st_count(st_end_pts) > 0 ) {

         if ( st_count(st_end_pts) > 5 )
         {
            int *pt;
            int *start_slice;
            st_table *st_pts = NULL;
            st_table *st_slice2stPts = st_init_table (st_ptrcmp, st_ptrhash);
            st_foreach_item(st_end_pts, stGen, (char **)&pt, (char **)&start_slice) {
               
               if ( !st_lookup(st_slice2stPts, ( char *)(start_slice), (char **)&st_pts) ) {
                  st_pts = st_init_table(st_numcmp, st_numhash);
                  st_insert(st_slice2stPts, (char *)start_slice, (char *)st_pts);
               }
               st_insert(st_pts, (char *)pt, (char *)1 );
            }
            
            // construct the REGION data structure
            ksd_array_to_region(st_slice2stPts, slice_index, output_prefix, IR_fn);

            // free resource
            st_foreach_item(st_slice2stPts, stGen, (char **)&start_slice, (char **)&st_pts) {
               st_free_table(st_pts);
            }
            st_free_table(st_slice2stPts);
         }

         sprintf(fn, "%s/debug/image_regions_of_end_region_%s.png", output_prefix, num_str);
         draw_2D_region_for_hash(fn, st_end_pts);
      }
      if ( db && st_count(st_new_pts) > 0 ) {
         sprintf(fn, "%s/debug/image_regions_of_active_region_by_the_end_of_%s.png", output_prefix, num_str);
         draw_2D_region_for_hash(fn, st_new_pts);
      }
      
      free_image(im_thin);
      free_image(im_orig);
      st_free_table(st_end_pts);
   }

   // Now we have done all the keyslices computation, let's put the remaining active
   // regions into final report.
   {
      // show for each point in st_active_data
      // group the points based on the start_slice

      int *pt;
      int *start_slice;
      st_table *st_slice2stPts = st_init_table (st_ptrcmp, st_ptrhash);
      st_foreach_item(st_active_data, stGen, (char **)&pt, (char **)&start_slice) {

         st_table *st_pts = NULL;
         if ( !st_lookup(st_slice2stPts, ( char *)(start_slice), (char **)&st_pts) ) {
            st_pts = st_init_table(st_numcmp, st_numhash);
            st_insert(st_slice2stPts, (char *)start_slice, (char *)st_pts);
         }
         st_insert(st_pts, (char *)pt, (char *)1 );
      }

      if ( db ) {
         sprintf(fn, "%s/debug/image_regions_of_final_active.png", output_prefix);
         draw_3D_region_for_each_layer(fn, st_slice2stPts);
      }

      {
         // call BPA to generate the boundary
         // construct the REGION data structure
         ksd_array_to_region(st_slice2stPts, right_num, output_prefix, IR_fn);
      }

   }
}

#else
void ksd_with_segmentation(void **params)
{

   // initialize the global vars
   image_width = -1;
   image_height = -1;
   
   // some thresholds for comparison and matching
   /*
     Here we have 3 thresholds for segementation and keyslices detection.
     To merger all these 3 threshold, the algorithm is based on the observation:
     1 - 0.9 -0.8; 5 - 0.5 - 0.4; 9 - 0.1 - 0.1
     We can choose a unique threshold \tau_d to be [0, 1].
     thres_mask would be (int)(10*\tau_d); => [0, 10];
     thres_region = thres_data = (1 - \tau_d) => [1, 0];  // what does 0 matched mean :P
     we should have a way to set these 3 parameters separately.
    */
   int   thres_mask   = 3;   /* mask for region */
   float thres_region = 0.5; /* matched region ratio */
   float thres_data   = 0.4; /* matched data ratio */
   //const float thres_region = 0.9; /* matched region ratio */
   //const float thres_data   = 0.8; /* matched data ratio */
   
   // decode the parameters
   bool db = true; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_prefix  = (char *) params[3];
   char *IR_fn = (char *) params[4];
   thres_mask   = (int) params[5];
   thres_region = *((float *) params[6]);
   thres_data   = *((float *) params[7]);
   float tau_d  = *((float *) params[8]);
   int  seg_app = (int) params[9];

   if ( tau_d >= 0.0 ) {
      assert( tau_d <= 1.0 );
      thres_mask = (int) ( 10 * tau_d );
      thres_region = 1 - tau_d;
      thres_data = thres_region;
      printf("The thresholds are, thres_mask: %d, thres_region: %f, thres_data: %f\n", thres_mask, thres_region, thres_data);
   }

   assert( input_prefix );
   assert( output_prefix );

   // clean file if exist
   char clean_IR_fn[1000];
   sprintf(clean_IR_fn, "%s/%s", output_prefix, IR_fn);
   FILE *fd_out = fopen(clean_IR_fn, "w");
   assert(fd_out);
   fclose(fd_out);

   char *fn_ext = NULL;
   {  // let's guess the image extension file.
      char fn[1000];
      sprintf(fn, "%s_0100.tif", input_prefix);
      if ( !file_exist(fn) ) {
         fn_ext = strdup("png");
      } else {
         fn_ext = strdup("tif");
      }
   }
   
   // start the computation
   int k;
   array_t *gbl_active_regions = array_alloc(KSD_REGION *, 0);
   array_t *arr_log_file = array_alloc(vector<int> *, 0);
   st_generator *stGen;
   vector<int> *log_region;
   BYTE **new_im = NULL, **ref_im = NULL;
   for (int i = left_num; i < right_num; i ++ )
   {
      st_table *st_all_regions = NULL;  // all the region pixels including data
      st_table *st_all_data    = NULL;  // all the data only
      st_table *st_region = NULL;       // for each individual region
      st_table *st_data   = NULL;       // for each data structure.

      char fn[1000];
      char *num_str = digit_string(4, i);
      sprintf(fn, "%s_%s.%s", input_prefix, num_str, fn_ext);

      printf("INFO: computing %s\n", fn);
      
      // compute the regions.
      st_table *pixel2rid = st_init_table(st_ptrcmp, st_ptrhash);
      comp_segments(fn, st_all_regions, st_all_data, new_im, pixel2rid, thres_mask, seg_app);

      // check whether this is a blank image.
      if ( st_count(st_all_regions) == 0 ) {
         free_image(new_im);
         st_free_table(st_all_data);
         st_free_table(st_all_regions);
         st_free_table(pixel2rid);
         continue;
      }

      // draw the segmentation regions for debug
      if ( 1 || db ) {
         sprintf(fn, "%s/debug/image_regions_of_slice_%s.png", output_prefix, num_str);
         draw_3D_region_from_hash(fn, st_all_regions, st_all_data);
      }
      
      if ( i == left_num || array_n(gbl_active_regions) == 0 ) {

         // init the active table T.
         printf("Init the reference due to 0 size of gbl_active_regions\n");
         st_foreach_item(st_all_regions, stGen, (char **)(&st_region), (char **)(&st_data)) {
            assert(st_region);
            assert(st_data);
            KSD_REGION *region = new_ksd_region(i);
            hash_2_vector(st_region, region->data_region);
            hash_2_vector(st_data, region->data_structure); 
            array_insert_last(KSD_REGION *, gbl_active_regions, region);
         }

         ref_im = new_im;
         st_free_table(st_all_data);
         st_free_table(st_all_regions);
         st_free_table(pixel2rid);
         continue;
      }

      // draw the active regions for debug
      if ( db ) {
         sprintf(fn, "%s/debug/image_regions_of_slice_%s_active.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, gbl_active_regions);
      }
      
      // region-based comparison.
      st_table *st_matched_regions = st_init_table(st_ptrcmp, st_ptrhash);
      array_t *arr_unmatched_active_regions = array_alloc(KSD_REGION *, 0);
      array_t *arr_unmatched_new_regions    = array_alloc(KSD_REGION *, 0);
      array_t *new_active_regions = array_alloc(KSD_REGION *, 0);
      KSD_REGION *region;
      arrayForEachItem(KSD_REGION *, gbl_active_regions, k, region) {
         bool matched_data = false;
         
         // find the matched region
         vector<int> new_region;
         st_table *st_new_region = NULL;
         if ( find_matched_region (region, st_all_regions, new_region, st_new_region, pixel2rid, thres_region) &&
              find_matched_structure(region->data_structure, new_region, ref_im, new_im, thres_data) ) {
               matched_data = true;
         }

         // if !find_matched_region -> no matched data
         // or !find_matched_structure (no matched data)
         if ( !matched_data ) {

            // dump this region to log file (end of a sub region)
            vector<int> *log_region = new vector<int>();
            region->end = i;
            log_region->push_back(region->rid);
            log_region->push_back(region->start);
            log_region->push_back(region->end);
            array_insert_last(vector<int> *, arr_log_file, log_region);

            // call BPA to generate the boundary
            ksd_generate_polygon(region, output_prefix, IR_fn, db);
            array_insert_last(KSD_REGION *, arr_unmatched_active_regions, region);
            
         } else {
            st_insert(st_matched_regions, (char *)st_new_region, (char *) 1);
            array_insert_last(KSD_REGION *, new_active_regions, region);
         }
      }

      // check the new regions of the new image that is not matched by any reference region.
      st_foreach_item(st_all_regions, stGen, (char **)&st_region, (char **)&st_data) {
         if (!st_is_member(st_matched_regions, (char *)st_region)) {
            assert(st_region);
            assert(st_data);
            KSD_REGION *region = new_ksd_region(i);
            hash_2_vector(st_region, region->data_region); 
            hash_2_vector(st_data, region->data_structure); 
            array_insert_last(KSD_REGION *, new_active_regions, region);
            array_insert_last(KSD_REGION *, arr_unmatched_new_regions, region);
         }
      }

      // draw the active regions for debug
      if ( db ) {
         sprintf(fn, "%s/debug/image_regions_of_slice_%s_unmatched_active.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, arr_unmatched_active_regions);
         sprintf(fn, "%s/debug/image_regions_of_slice_%s_unmatched_new_region.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, arr_unmatched_new_regions);
      }
      
      // update ref_im for comparison of next step
      if (ref_im) {
         free_image(ref_im);
         ref_im = NULL;
      }
      array_free(gbl_active_regions);
      gbl_active_regions = new_active_regions;
      array_region_to_image(ref_im, gbl_active_regions);

      // free resource for each iteration
      free(num_str);
      free_image(new_im);
      st_free_table(st_matched_regions);
      st_free_table(pixel2rid);
      array_free(arr_unmatched_new_regions);

      // we should be able to reclaim the memory of regions in arr_unmatched_active_regions.
      KSD_REGION *tmp_region;
      arrayForEachItem(KSD_REGION *, arr_unmatched_active_regions, k, tmp_region) {
         delete tmp_region;
      }
      array_free(arr_unmatched_active_regions);

      // reclamin st_all_region, st_all_data
      st_foreach_item(st_all_regions, stGen, (char **)&st_region, (char **)&st_data) {
         st_free_table(st_region);
         st_free_table(st_data);
      }
      st_free_table(st_all_data);
      st_free_table(st_all_regions);
   }

   // Now we have done all the keyslices computation, let's put the remaining active
   // regions into final report.
   {
      KSD_REGION *region;
      arrayForEachItem(KSD_REGION *, gbl_active_regions, k, region) {
         vector<int> *log_region = new vector<int>();
         region->end = right_num - 1;
         log_region->push_back(region->rid);
         log_region->push_back(region->start);
         log_region->push_back(region->end);
         array_insert_last(vector<int> *, arr_log_file, log_region);
         
         // call BPA to generate the boundary
         ksd_generate_polygon(region, output_prefix, IR_fn, db);
      }
   }

   // print out the final IR based on the log file?
   // This could be done during the processing of finding each individual local region.
   char region_fn[1000];
   sprintf(region_fn, "%s/debug/all_regions.txt", output_prefix );
   FILE *fd = fopen(region_fn, "wt");
   arrayForEachItem(vector<int> *, arr_log_file, k, log_region) {
      fprintf(fd, "%d %d %d\n", log_region->at(0), log_region->at(1), log_region->at(2));
   }
   fclose(fd);
}

#endif

/*
Function: bpa_update_contours

Add/Merge control points based on adjacent boundaries

Command:
     Project.exe -K -O "-f seg_ir.txt -A 2 -s 1000 -h 1024 -w 1024 -b 1 
     -P result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_seg_result_1000_1_1
     -O result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_seg_result_1000_1_1"
     
Parameters:
     passing through a 2-D void pointer.

Data structures:

Assumptions:

     The boundary of the lower layer is always more accurate than the higher layer.
     
Algorithms:
 
     There are several cases to consider:
     1. move a point
        this case is when a control point is very close to an previous existed control point,
        and its neighbor control points might be landing on existed control points.
        Note: leave tapered control points untouched - make sure this is not tapered/follow-me structure.
        
     2. add a point
        if there is one more control points existed between the two end points of existed boundary,
        let's add one more point. 
        
     3. drop a point
        the two neighbor points are close to existed neighbor points, there is no other control points in
        existed ones, let drop this one. This is a noise point.

Implementation:

     step 1. group segments based on the starting and ending slices.
     step 2. for each local segments, compute the adjacent boundaries, find lower and higher two extra boundaries.
     step 3. vote for some cases to avoid misleading by noise data.
*/
void bpa_update_contours(void **params)
{
   // decode the parameters
   bool db = true; // debug?
   int sample_rate   = gbl_opts->total_slices;
   char *input_path  = (char *) params[0];
   char *ir_fn0      = (char *) params[1];
   char *output_path = (char *) params[2];
   int   dist_thres  = (int   ) params[3];

   // step 1: group the segments from the ir representation
   char ir_fn[1000];
   sprintf(ir_fn, "%s/%s", input_path, ir_fn0);
   array_t *z_arr = array_alloc(vector<float> *, 0);
   array_t *end_points = load_end_points_from_IR(ir_fn, z_arr);
   assert(array_n(z_arr) == array_n(end_points));

   // for each slice containing new keyslice, let's draw the lines on this slice
   // and then for each new contour starting at this slice, draw the original lines with
   // a predefined random color. After updated, draw the new contour with the same 
   // predefined random color.

   // go through end_points, two hash table are created.
   // st_starting, st_ending
   st_table *st_starting = st_init_table(st_ptrcmp, st_ptrhash);
   st_table *st_ending   = st_init_table(st_ptrcmp, st_ptrhash);
   map<char *, int> mapLayer2Index;

   {
      array_t *layer;
      int k;
      arrayForEachItem(array_t *, end_points, k, layer) {
         vector<float> * height = array_fetch(vector<float> *, z_arr, k);

         // This float computation must be careful!!!!
         // WITHOUT add .5, two small float number may become the same.
         // for example, 0.075 0.076 are loaded into float number to be 0.075001 0.07599901
         // they will be the same after times 1000 and cast to integer number.
         int base = (int)((*height)[0] * sample_rate + .5);
         int top  = (int)((*height)[1] * sample_rate + .5);
         mapLayer2Index[(char *)layer] = top;
         assert( base != top );

         array_t *arr_base;
         array_t *arr_top ;
         if ( !st_lookup(st_starting, (char *)base, (char **)&arr_base) ) {
            arr_base = array_alloc(array_t *, 0);
            st_insert(st_starting, (char *)base, (char *)arr_base);
         }
         array_insert_last(array_t *, arr_base, layer);

         if ( !st_lookup(st_ending, (char *)top, (char **)&arr_top) ) {
            arr_top = array_alloc(array_t *, 0);
            st_insert(st_ending, (char *)top, (char *)arr_top);
         }
         array_insert_last(array_t *, arr_top, layer);
      }
   }


   // draw the image for each slice
   // for debug only ???
   if ( 0 )
   {
      array_t *arr_base = NULL, *arr_top = NULL;
      for (int i = 0; i < sample_rate; i ++ ) {
         if ( !st_is_member(st_starting, (char *)i) )
            continue;

         printf("Drawing slice %d...\n", i);
         
         st_lookup(st_starting, (char *)i, (char **)&arr_base);
         st_lookup(st_ending,   (char *)i, (char **)&arr_top);

         // draw top/old in red and draw new/base in random color
         {
            int mask = 1;
            BYTE ***im = new_3D_image(image_height, image_width);
            int r = 255, g = 0, b = 0;

            int k,j;
            array_t *layer;
            vector<int> *points;
            if ( arr_top && array_n(arr_top) > 0 ) {
               arrayForEachItem(array_t *, arr_top, k, layer) {
                  int x0, y0;
                  r = rand() % 255;
                  g = rand() % 255;
                  b = rand() % 255;
                  arrayForEachItem(vector<int> *, layer, j, points) {
                     int x = (*points)[0];
                     int y = (*points)[1];
                     for (int i = -mask; i < mask+1; i++)
                        for (int j = -mask; j < mask+1; j++)
                              set_color(im,y+i,x+j,r,g,b);
                     if ( j == 0 ) {
                        x0 = x;
                        y0 = y;
                        continue;
                     } else {
                        draw_line(im, image_height, image_width, x0, y0, x, y, r, g, b);
                        x0 = x;
                        y0 = y;
                     }
                  }
               }

               char fn[1000];
               sprintf(fn, "%s/image_slice_%d.png", output_path, i);
               Save2File(fn, NULL, NULL, image_width, image_height, 1, im);
               free_image_3D(im);
               im = new_3D_image(image_height, image_width);
            }

            if ( arr_base && array_n(arr_base) > 0 ) {
               arrayForEachItem(array_t *, arr_base, k, layer) {
                  r = rand() % 255;
                  g = rand() % 255;
                  b = rand() % 255;
                  int x0, y0;
                  arrayForEachItem(vector<int> *, layer, j, points) {
                     int x = (*points)[0];
                     int y = (*points)[1];
                     for (int i = -mask; i < mask+1; i++)
                        for (int j = -mask; j < mask+1; j++)
                           set_color(im,y+i,x+j,r,g,b);
                     if ( j == 0 ) {
                        x0 = x;
                        y0 = y;
                        continue;
                     } else {
                        draw_line(im, image_height, image_width, x0, y0, x, y, r, g, b);
                        x0 = x;
                        y0 = y;
                     }
                  }
               }
            }
            
            // save the image slice
            char fn[1000];
            sprintf(fn, "%s/image_slice_%d_new.png", output_path, i);
            Save2File(fn, NULL, NULL, image_width, image_height, 1, im);
            free_image_3D(im);

            /*
            static int count = 0;
            count ++;
            if (count > 10)
               exit(0);
            */
         }
      }
   }

   // let's handle a very basic situtation for now. Anyway, it could be extended easily
   // The case is if the new control points is far away from the lines, ignore. Otherwise,
   // move the point to the nearest point on the line or if possible, use the same control
   // point. No need to add/remove any control points at this point in time.
   // Once done, move on. (show a demo)
   {
      // this threshold should be updated on the fly based on the threshold of \tau_d in KSD.
      // int dist_thres = 4; // pass through paramter

      // this is not necessary, for diagonosal distance, it will count the way which can go most fast pixel
      // int dist_thres_cp = (int) (1.414 * dist_thres); 

      array_t *arr_base = NULL, *arr_top = NULL;
      bool is_first = true;
      map<int, int> mapLineSeg;
      map<int, int> mapVertice;
      for (int i = 0; i < sample_rate; i ++ )
      {
         if ( !st_is_member(st_starting, (char *)i) )
            continue;

         printf("Processing segments in slice %d...\n", i);
         
         st_lookup(st_starting, (char *)i, (char **)&arr_base);
         st_lookup(st_ending,   (char *)i, (char **)&arr_top);

         // use mapLineSeg<int, int> to create LUT for regular points up to distance dist_thres.
         // use mapVertice<int, int> to create LUT for control points up to distance dist_thres_cp.
         // for each new observed control point, first lookup mapVertice<P, xxx>, if found, replace
         // this control point P with xxx. Otherwise, lookup mapLineSeg<P, yyy> and replace.
         // if none of the maps can find P, leave P alone.
         // after change the control points, we should update the contour so that the next layer will
         // refer to the new contour and therefore avoid collapsing.
         {
            int k,j;
            array_t *layer;
            vector<int> *points;
            if ( arr_top && array_n(arr_top) > 0 ) {
               arrayForEachItem(array_t *, arr_top, k, layer) {
                  int x0, y0;
                  arrayForEachItem(vector<int> *, layer, j, points) {
                     int x = (*points)[0];
                     int y = (*points)[1];
                     mapVertice[index(x,y)] = 1;
                     if ( j == 0 ) {
                        x0 = x; y0 = y;
                        continue;
                     }
                     vector<int> *ptsOnLine = pixels_between_2_points_in_order(image_height, image_width, x0, y0, x, y);
                     for (int i = 0; i < ptsOnLine->size(); i ++)
                        mapLineSeg[(*ptsOnLine)[i]] = 1;

                     x0 = x;
                     y0 = y;
                  }
               }

               // for the line structure like the following:
               // *-----------*(P)
               // *(A)-------*(B)---*(C)
               // 0) If the substitution point P is the same as A, remove B.
               // 1) If B is not close to any vertice point P, but close to a line point P', replace B with P'
               //    No matter whether A is close to any point or not. 
               // 2) If B is close to a vertice point P:
               //        a) If A is replaced with P, remove B
               //        b) If A is not replaced with P, replace B with P
               //        c) If A is null (B is the starting point), replace B with P.
               // 3) If # of points in the segment <layer> is less than 2, remove this segment.
               // 4) If B is close to nothing (did not find any nearby point), keep B.
               //
               array_t *arr_new = array_alloc(array_t *, 0);
               arrayForEachItem(array_t *, arr_base, k, layer) {  // for each segment in the slice i
                  int point_A = -1;
                  array_t *new_layer = array_alloc(vector<int> *, 0);
                  arrayForEachItem(vector<int> *, layer, j, points) {  // for each polygon in the segment k
                     int x = (*points)[0];
                     int y = (*points)[1];
                     int B = index(x, y);

                     // compute the neighbor of B based on dist_thres
                     int sub_pt = -1;
                     bool found = false;
                     bool found_lp = false;
                     {
                        int mask = 0;
                        while ( mask < dist_thres + 1 ) // search from closer to further
                        {
                           int i_y = y - mask;
                           for (int i_x = x - mask; i_x < mask + x + 1; i_x ++) {
                              if ( i_x > -1 && i_y > -1 && i_x < image_width && i_y < image_height ) {
                                 int loc = index(i_x, i_y);
                                 if ( mapVertice.find(loc) != mapVertice.end() ){
                                    sub_pt = loc;
                                    found = true;
                                    break;
                                 }

                                 // check line point; don't break in the outer while loop if find one
                                 // since vertice point has higher priority.
                                 if ( !found_lp && mapLineSeg.find(loc) != mapLineSeg.end() ) {
                                    sub_pt = loc;
                                    found_lp = true;
                                    break;
                                 }
                              }
                           }

                           if (found) break;
                           
                           i_y = y + mask;
                           for (int i_x = x - mask; i_x < mask + x + 1; i_x ++) {
                              if ( i_x > -1 && i_y > -1 && i_x < image_width && i_y < image_height ) {
                                 int loc = index(i_x, i_y);
                                 if ( mapVertice.find(loc) != mapVertice.end() ){
                                    sub_pt = loc;
                                    found = true;
                                    break;
                                 }

                                 // check line point; don't break if find one since vertice point has higher priority.
                                 if ( !found_lp && mapLineSeg.find(loc) != mapLineSeg.end() ) {
                                    sub_pt = loc;
                                    found_lp = true;
                                    break;
                                 }
                              }
                           }

                           if (found) break;

                           mask ++;
                        }
                     }

                     // check the result as the alg described above
                     if ( found || (!found && found_lp) ) { //. case 1) and 2)
                        if ( sub_pt != point_A ) { // otherwise remove B as desribed above: case 0)
                           int p_x, p_y;
                           assert( sub_pt > -1 );
                           p_x = x_axis(sub_pt);
                           p_y = y_axis(sub_pt);
                           vector<int> *new_point = new vector<int> ();
                           new_point->push_back(p_x);
                           new_point->push_back(p_y);
                           array_insert_last(vector<int> *, new_layer, new_point);
                           point_A = sub_pt;
                        }
                     }
                     else { // keep B for case 4)
                        vector<int> *new_point = new vector<int> ();
                        new_point->push_back(x);
                        new_point->push_back(y);
                        array_insert_last(vector<int> *, new_layer, new_point);
                     }
                     
                  }
                  
                  // add this layer to new arry if its size is at LEAST 2: case 3)
                  if ( array_n(new_layer) > 1 ) {
                     array_insert_last(array_t *, arr_new, new_layer);
                     assert(mapLayer2Index.find((char *)layer) != mapLayer2Index.end());
                     assert( mapLayer2Index[(char *)layer] != i );
                     mapLayer2Index[(char *)new_layer] = mapLayer2Index[(char *)layer];
                  } else {
                     array_free(new_layer);
                  }
               }

               // let's move the content of arr_new to arr_base. We cannot simply assign arr_new to arr_base,
               // because the pointer in the hash table of top/base is not changed.
               {
                  // first empty arr_base,
                  while ( array_n(arr_base) > 0 ) {
                     array_remove_last(arr_base);
                  }
                  
                  // then move element from arr_new to arr_base.
                  arrayForEachItem(array_t *, arr_new, k, layer) {
                     array_insert_last(array_t *, arr_base, layer);
                  }

                  array_free(arr_new);
               }

               // clear mapLineSeg and mapVertice
               mapLineSeg.clear();
               mapVertice.clear();
               
            } // end of if arr_top
         }

      }

      // update the IR file with new boundary of the segments
      // the z_arr info is not changed ,only the points in layer are changed.
      {
         char new_IR_fn[1000];
         char *prefix_fn = strdup(ir_fn0);
         prefix_fn[strlen(prefix_fn)-4] = 0;
         sprintf(new_IR_fn, "%s/merge_%d_%s.txt", output_path, dist_thres, prefix_fn);
         FILE *updated_fd = fopen(new_IR_fn, "wt");
         assert(updated_fd);

         array_t *arr_base = NULL, *arr_top = NULL;
         for (int i = 0; i < sample_rate; i ++ )
         {
            if ( !st_is_member(st_starting, (char *)i) )
               continue;
            st_lookup(st_starting, (char *)i, (char **)&arr_base);
            assert(arr_base);

            int k;
            array_t *layer;
            arrayForEachItem(array_t *, arr_base, k, layer) {
               assert( mapLayer2Index.find((char *)layer) !=  mapLayer2Index.end() );
               assert( array_n(layer) > 1 );

               // dump the begin information
               fprintf( updated_fd, "BEGIN POLYGON\n" );
                  
               int top = mapLayer2Index[(char *)layer];

               vector<int> *points;
               int j;
               arrayForEachItem(vector<int> *, layer, j, points) {
                  fprintf( updated_fd, "%d %d\n", (*points)[0], (*points)[1] );
               }

               // dump the end information
               fprintf( updated_fd, "EXTRUSION\n" );
               fprintf( updated_fd, "HEIGHT\n" );
               fprintf( updated_fd, "%f %f\n", (float)i/sample_rate, (float)top/sample_rate );
               fprintf( updated_fd, "END POLYGON\n" );
            }
         }
         
      }

   } // empty   
}

/*
Function: comp_segments_image

Input:
     An image and threshold for segmentation

Output:
     Two hash tables containing the segments information.
     st_all_regions:   the segments and its regions defined by the threshold
     st_all_data   :   the real segment data

*/
void comp_segments_image(BYTE **im, st_table *&st_all_regions, st_table *&st_all_data, int thres_mask, int approach)
{
   BYTE **I = copy_image(im, image_height, image_width);
   st_table *pixel2rid = st_init_table(st_ptrcmp, st_ptrhash);

   while ( true ) {
      
      // s2: pick up a starting point $p$ from image I
      int p_index = comp_seg_starting_point(I);
      if ( p_index == -1 )
         break;
      
      // s3: dilating $p$ with mask $m=threshold$, collect new points set $s$.
      // s4: if $s$ is not empty, repeat s3 on points in $s$.
      //     if $s$ is empty, update $I'$ containing dilated pixels, set value 0.
      //                      update $I = I && I'$;
      st_table *st_new_data   = st_init_table(st_ptrcmp, st_ptrhash);
      st_table *st_new_region;
      
      if ( approach == 0 )  // graph cut based
         st_new_region = comp_seg_dilate_for_region(I, p_index, thres_mask, st_new_data, pixel2rid);
#ifndef LINUX      
      else
         st_new_region = comp_seg_gc_for_region(I, p_index, thres_mask, st_new_data, pixel2rid);
#endif      
      
      st_insert(st_all_regions, (char *)st_new_region, (char *) st_new_data);
      st_insert(st_all_data, (char *)st_new_data, (char *) st_new_region);

      // s5: if $I$ is empty, done, otherwise, go to s2.
      // s2 will check p_index
   }
   free_image(I);
   st_free_table(pixel2rid);
}


/*
Function: ksd_follow_me_detect

Infer the follow-me structure for keyslice detection

Command:
     Project.exe -K -O "-f image_slice_0425_based_mask.png -A 3 -s 1000 -b 8 -F ir_followme.txt
     -P result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_seg_result_1000_1_1
     -O result/point_cloud_interior_GCT_1000/slices_1000_640_800_bottom_up_seg_result_1000_1_1_result"

Command to obtain final model:
     Project.exe -j -O "-p result/point_cloud_HT_1000/slices_1000_512_1024_left_right_result/ir_fm.txt
     -P result/point_cloud_HT_1000/ir_fm.iv -t 12 -w 512 -M 1 -x -9.17 -X 13.06 -y -8.05 -Y 15.40 -z -1.04 -Z 10.50
     -D 1 -o 2 -W 640 -f -1 2"

Parameters:
     passing through a 2-D void pointer.

Data structures:

Assumptions:

     We can handle the modeling of intersection of two random extrusions: two walls intersect each other, and
     the extrusions beyond each other are eliminated automatically.

Algorithms:
 
     There are several cases to consider:
     1. how to know whether there is follow-me structure existing
        
     2. what kind of FM to infer
        the template of the structure may change as the FM moves, e.g. size is reduced, a bar is
        moving as FM moves.
        
     3. how to represent FM structure in IR
        represents the whole base structure in polygon, and then, define the trajactory of the base pattern
        by a series of (x, y) points. The whole 3D FM model can be reconstructed by translating the base
        pattern along the trajactory.

Ideas:
     1. to get the template structure and avoid broken line, do the followings: a. get the segments with relative
        large threshold. b. apply BPA on each segment. c. integrate all contours of each segment to get the template
        to avoid broken lines.
     
Implementation:

     step 1. manually mask the template(big, whole @slice_0425.png) for the follow-me structure, choose the reference number of the slice.
     step 2. match the template in two directions (left/right) in a small region (10x10, 20x20). If the match ratio is
             bigger than a threshold, log down the offset/translation of the pattern. If not, a new base pattern is assumed
             to be found, and use the new pattern as template.
     step 3. log down each base template/contour and its trajactory, reconstruct the whole model by concatenating all the
             base templates. The smaller the threshold is, the more accurate model we will obtain.
*/
void ksd_follow_me_detect(void **params)
{
   // decode the parameters
   VERBOSE_LEVEL db  = KEY_INFO; // debug?
   int sample_rate   = gbl_opts->total_slices;
   char *input_path  = (char *) params[0];
   char *fn0         = (char *) params[1]; // mask image;
   char *output_path = (char *) params[2];
   int   dist_thres  = (int   ) params[3];
   char *ir_fn       = (char *) params[4];
   int   seg_app     = (int   ) params[5];

   int h, w;
   char fn[1000];
   BYTE **mask_im = NULL, **cur_im = NULL, **new_im = NULL, **bpa_im = NULL;
   st_table *st_all_regions = st_init_table(st_ptrcmp, st_ptrhash);
   st_table *st_all_data = st_init_table(st_ptrcmp, st_ptrhash);
   char *ds; // digit number string
   int cur_base_num = -1;

   // step 0: match process on the image created by BPA in an iterative way
   {
      // we need couple of thresholds
      int step_num = 4;          // so we do NOT want to check one after another, too little change
      int cmask = 5;             // we will check locally in cmask x cmask;
      float match_ratio = 0.5;     // the ratio deemed as matched.

      int ref_num = 425;         // the reference number

      // do the comparison in a function with direction as parameter
      int dir = 1;
      int end_num = 550; //ending   point
      int beg_num = 143; //starting point
      //int beg_num = 234; //starting point

      bool need_calibration = true;
      sprintf(fn, "%s/%s", output_path, ir_fn);
      FILE *fd = fopen(fn, "wt");
      assert(fd);
      
      for (int  i = ref_num; true; i += dir*step_num )
      {
         
         // step 1: get the cleared image from the mask image
         {
            if ( need_calibration ) {
               // dump the head of IR
               fprintf(fd, "BEGIN POLYGON\n");
               cur_base_num = i;
               
               // free bpa_im only when we need to change the reference image/find a keyslice
               if ( bpa_im ) {
                  free_image(bpa_im);
                  bpa_im = NULL;
               }
            }

#ifdef LINUX
            CImg<BYTE> *orig_img = NULL;
#else
            Bitmap *orig_img = NULL;
#endif

            if ( i == ref_num ) {

               printf("INFO: get the mask image at point %d\n", i );
               
               sprintf(fn, "%s/%s", output_path, fn0);
               orig_img = load_image(fn, h, w);
               image_height = h;
               image_width  = w;

               // free mask image only when we need to change mask image
               if ( mask_im ) {
                  free_image(mask_im);
                  mask_im = NULL;
               }

               set_image(mask_im, orig_img);
               delete orig_img;
            }

            assert(mask_im);

            ds = digit_string(4, i);
            sprintf(fn, "%s/image_slice_%s.png", input_path, ds);
            orig_img = load_image(fn, h, w);

            set_image(cur_im, orig_img);
            delete orig_img;

            // let's clear the cur_im based on mask image
            if ( new_im )
               free_image(new_im);
            
            new_im = new_image(h, w);
            int total = w * h;
            for (int i = 0 ; i < total; i ++)
               new_im[0][i] = mask_im[0][i] & cur_im[0][i];

            if (db > KEY_INFO) {
               sprintf(fn, "%s/slice_%s_cleared.png", output_path, ds);
               Save2File(fn, new_im, NULL, image_width, image_height, 0, NULL, 0); // 1: tif
            }

            free_image(cur_im);
         }

         // step 2: compute the segmentation
         if ( need_calibration )
         {
            printf("INFO: compute the segments at point %d\n", i );
            
            {
               if ( st_count (st_all_regions) > 0 ) {
                  st_free_table(st_all_regions);
                  st_free_table(st_all_data);
                  st_all_regions = st_init_table(st_ptrcmp, st_ptrhash);
                  st_all_data = st_init_table(st_ptrcmp, st_ptrhash);
               }

               comp_segments_image(new_im, st_all_regions, st_all_data, dist_thres, seg_app);

               // draw the segmentation regions for debug
               if ( db > KEY_INFO ) {
                  sprintf(fn, "%s/slice_%s_segments.png", output_path, ds);
                  draw_3D_region_from_hash(fn, st_all_regions, st_all_data);
               }
            }
         }

         // step 3: compute BPA on each segment to get the connected contour of each segment with holes filled.
         // this is to handle the cases where the curve are formed with broken lines.
         if ( need_calibration )
         {

            printf("INFO: compute the BPA at point %d\n", i );
            
            // set the parameters for BPA
            {
               gbl_opts->BPA_parameter_file = NULL;
               gbl_opts->BPA_need_do_refinement = 1;
               gbl_opts->BPA_need_do_HT_BPA_combination = 0;
               gbl_opts->radius = dist_thres;
               gbl_opts->radius_min = 1;
               gbl_opts->BPA_debug_mode = 0;
               gbl_opts->BPA_debug_image_interval = 100;
               gbl_opts->BPA_clear_first_circle_data = 1; // otherwise, seg_fault when copy_image, size is changed.
               gbl_opts->BPA_do_not_remove_extra_turning_around_pts = 1; // new option to keep the whole points.
               gbl_opts->BPA_do_not_check_turning_around = 0;
               gbl_opts->BPA_starting_point_sweeping_dir = 1;
               gbl_opts->BPA_starting_point_sweeping_cut_off = image_width + (8+gbl_opts->radius*4); //2 * offset (2*rad + 4)
               gbl_opts->BPA_check_turning_point_for_first_iteration = 1; // check turning around
               gbl_opts->BPA_turning_around_angle = 60;
            }

            // call BPA
            st_table *st_data;
            st_generator *sgOut, *sgInner;
            int counter = 0;
            int im_backup_h = image_height;
            int im_backup_w = image_width;
            array_t *arr_all_boundaries = array_alloc(array_t *, 0);
            st_foreach_item( st_all_data, sgOut, (char **)&st_data, NULL ) {
               assert(st_data);
               image_height = im_backup_h;
               image_width  = im_backup_w;
               BYTE **seg_im = new_image(image_height, image_width);

               int *p_index;
               st_foreach_item( st_data, sgInner, (char **)&p_index, NULL ) {
                  seg_im[0][(int)p_index] = BLACK;
               }
         
               // draw the image;
               if (db > LOW_INFO) {
                  sprintf(fn, "%s/image_regions_of_before_BPA_%d.png", output_path, counter);
                  Save2File(fn, seg_im, NULL, image_width, image_height, 0, NULL);
               }
         
               array_t *boundary_array = array_alloc(deque<int> *, 0);
               BPA_boundary_image(seg_im, boundary_array);

               // draw the image;
               if (db > LOW_INFO) {
                  sprintf(fn, "%s/image_regions_of_BPA_%d.png", output_path, counter++);
                  draw_boundary_image(boundary_array, NULL, gbl_opts->radius, fn);
               }

               array_insert_last(array_t *, arr_all_boundaries, boundary_array);
         
               // free boundary_array;
               free_image(seg_im);
            }

            // draw the final image;
            bpa_im = draw_boundaries_on_image(arr_all_boundaries, gbl_opts->radius);
            
            // update the IR file
            dump_boundaries_to_file(arr_all_boundaries, gbl_opts->radius, fd);
            fprintf(fd, "FOLLOW-ME\n");
            
            // After BPA, the image_height and image_width have been changed.
            image_height = im_backup_h;
            image_width  = im_backup_w;

            if (db > KEY_INFO) {
               sprintf(fn, "%s/slice_%s_all_BPA_regions.png", output_path, ds);
               Save2File(fn, bpa_im, NULL, image_width, image_height, 0, NULL);
            }


         }

         assert(bpa_im);
         assert(mask_im);
         assert(new_im);

         if ( need_calibration ) {
            need_calibration = false;
            continue;
         }

         // step 4: do the matching between new_im and bpa_im
         // we should match from new_im to bpa_im. It is ok new_im is much smaller than bpa_im
         // since we assume we can handle this case later using a machanism that prevent extrusion
         // from penetrating a wall.
         {
            int max_matched_points = 0;
            int max_offset_x = 0 , max_offset_y = 0;
            int data_num = 0;

            int total_num = image_width * image_height;
            for (int j = 0; j < total_num; j ++)
               if ( new_im[0][j] == BLACK )
                  data_num ++;
            
            for (int x = -cmask; x <= cmask; x ++ )
               for (int y = -cmask; y <= cmask; y ++)
               {
                  // virtually translate the new_im;
                  // compute the matched points
                  int matched_points = 0;
                  for (int x0 = 0; x0 < image_width; x0 ++)
                     for (int y0 = 0; y0 < image_height; y0 ++)
                     {
                        int x_l = x0 + x;
                        int y_l = y0 + y;
                        if ( x_l >= 0 && x_l < image_width && y_l >= 0 && y_l < image_height
                             && new_im[y_l][x_l] == BLACK && bpa_im[y0][x0] == BLACK )
                           matched_points ++;
                     }

                  if ( matched_points > max_matched_points ) {
                     max_offset_x = x;
                     max_offset_y = y;
                     max_matched_points = matched_points;
                  }

                  // compare the result
               }

            // check the valibility of the matching
            float max_matched_ratio = (float)max_matched_points / (float)data_num;
            if ( max_matched_ratio >= match_ratio ) {
               
               printf("INFO: matched slice %d with ratio %f\n", i, max_matched_ratio);


               if ( abs(max_offset_x) + abs(max_offset_y) > cmask )
               {
                  
                  fprintf(fd, "%d %d %f\n", max_offset_x, max_offset_y, (float)( i+dir*step_num - cur_base_num)/sample_rate);
                  
                  // let's move bpa_im based on the offset???
                  BYTE **bpa_tmp = new_image(image_height, image_width);
                  for (int x0 = 0; x0 < image_width; x0 ++)
                     for (int y0 = 0; y0 < image_height; y0 ++) {
                        int x_l = x0 + max_offset_x;
                        int y_l = y0 + max_offset_y;
                        if ( x_l >= 0 && x_l < image_width && y_l >= 0 && y_l < image_height
                             && bpa_im[y0][x0] == BLACK )
                           bpa_tmp[y_l][x_l] = BLACK;
                  }
                  free(bpa_im);
                  bpa_im = bpa_tmp;
                  
                  if (db > KEY_INFO) {
                     sprintf(fn, "%s/slice_%s_ref_im.png", output_path, ds);
                     Save2File(fn, bpa_im, NULL, image_width, image_height, 0, NULL);
                  }
                  
               }
               
            } else {

               printf("INFO: unmatched slice %d with ratio %f, a new keyslice is found!\n", i, max_matched_ratio);
               float hei = cur_base_num/(float)sample_rate;
               fprintf(fd, "%d %d %f\n", max_offset_x, max_offset_y, (float)(i+dir*step_num-cur_base_num)/sample_rate);
               fprintf(fd, "HEIGHT\n%f %f\nEND POLYGON\n", hei, hei + 0.005);

               need_calibration = true;
               // find a new keyslice and redo the above steps???
            }
            
         }


         // switch direction if needed
         if ( dir > 0  && abs (i - end_num) < step_num) {

            if ( !need_calibration) {
               float hei = cur_base_num/(float)sample_rate;
               fprintf(fd, "0 0 %f\n", (float)step_num*2/sample_rate);
               fprintf(fd, "HEIGHT\n%f %f\nEND POLYGON\n", hei, hei + 0.005);
            }
            
            dir = -1;
            i = ref_num - dir*step_num;
            need_calibration = true;

         } else if ( dir < 0 && abs( i - beg_num ) < step_num ) {
            break;
         }
         
      } // end of for

      if ( !need_calibration ) {
         float hei = cur_base_num/(float)sample_rate;
         fprintf(fd, "0 0 %f\n", (float)step_num*2/sample_rate);
         fprintf(fd, "HEIGHT\n%f %f\nEND POLYGON\n", hei, hei + 0.005);
      }
      
      fclose(fd);
      
   } // end of empty

}


int ksd_matched_centroid(KSD_REGION *region,
                         st_table *st_region_centroid,
                         int threshold,
                         st_table *&st_ctrd_matched_region)
{
   assert(region);
   assert(st_region_centroid);

   int min_distance = 1000;

   int x0 = x_axis(region->centroid);
   int y0 = y_axis(region->centroid);

   st_generator *stGen;
   st_table *st_reg;
   int *cent;
   st_foreach_item(st_region_centroid, stGen, (char **)&st_reg, (char **)&cent) {
      int x = x_axis((int)cent);
      int y = y_axis((int)cent);

      if ( abs(x - x0) + abs(y - y0)  < min_distance ) {
         min_distance = abs(x - x0) + abs(y - y0);
         st_ctrd_matched_region = st_reg;
      }
      
   }

   return ( min_distance < threshold ) ? 1 : 0;
}

st_table *ksd_compute_centroid_all_regions(st_table *st_all_regions)
{
   st_table *st_reg2centroid = st_init_table(st_ptrcmp, st_ptrhash);

   st_generator *stGen;
   st_table *st_region, *st_data;
   st_foreach_item(st_all_regions, stGen, (char **)&st_region, (char **)&st_data) {

      assert(st_region);
      assert(st_data);
      
      st_generator *stGen0;
      int *point;
      long long sum = 0;
      st_foreach_item(st_data, stGen0, (char **)&point, (char **)NULL) {
         sum += (int)point;
      }

      int centroid = (int)((double)sum/st_count(st_data) + 0.5);
      st_insert(st_reg2centroid, (char *)st_region, (char *)centroid);
   }

   return st_reg2centroid;
}

void ksd_compute_centroid_for_region(KSD_REGION *region)
{
   assert(region);
   assert(region->data_structure.size() > 0 );

   // to compute the centroid, it is equivalent to do individual x,y
   // or to do the index(x, y)
   long long sum = 0;
   int total_pts = region->data_structure.size();
   for (int i = 0; i < total_pts; i ++ )
      sum += region->data_structure[i];

   region->centroid = (int)((double)sum/total_pts + 0.5);
   /*
   printf("The summation for the region is huge, the centroid: %d ->(%d, %d), [w,h]->[%d,%d]\n",
          region->centroid,
          x_axis(region->centroid),
          y_axis(region->centroid),
          image_width,
          image_height);
   */
   
}

// 1: taper-to-line
// 0: taper-to-point
int  ksd_taper_compute_type(KSD_REGION *last_region, array_t *taper_arr, int index)
{

   int matched_times = 0;
   int decision_times = 3;
   
   int k;
   KSD_REGION *region;
   arrayForEachItem(KSD_REGION *, taper_arr, k, region) {
      // dilate the base image by 1, count how many pixels are overlapped.
      // if for the consective regions, more than 50% with more than 10%, deemed as taper to line;
      // otherwise, taper to point.
      {
         if ( matched_times > decision_times )
            return 1;

         if ( k == array_n(taper_arr) - 1)
            break;

         KSD_REGION *top_region = array_fetch(KSD_REGION *, taper_arr, k + 1);
         assert(top_region);
         
         // convert vector to image:
         BYTE **image_base = NULL;
         vector_region_to_image(image_base, region->data_structure);
         image_dilation(image_base);

         int matched_pts = 0;
         for (int i = 0; i < top_region->data_structure.size(); i ++ ) {
            if ( image_base[0][top_region->data_structure[i]] == BLACK )
               matched_pts ++;
         }

         if ( (float) matched_pts / (float)top_region->data_structure.size() > 0.1 )
            matched_times ++;

         free_image(image_base);
      }
   }

   return 0;
}

array_t *ksd_taper_compute_each_region_control_points(array_t *taper_arr,
                                                      int &num_ctrl_pts,
                                                      int N)
{
   // generate image, call BPA, get the points.
   char fn[1000];
   array_t *arr_return  = array_alloc(array_t *, 0);
   st_table *st_counter = st_init_table(st_numcmp, st_numhash);
   int max_count = 0;
   
   int k;
   KSD_REGION *region;
   arrayForEachItem(KSD_REGION *, taper_arr, k, region ) {
      if ( region->data_structure.size() == 0)
         continue;
      
      ksd_generate_polygon(region, gbl_opts->output_prefix, NULL, true);

      sprintf(fn, "%s/region_%d.ply", gbl_opts->output_prefix, region->rid);

      if ( !file_exist(fn) ) {
         printf("WARNING: ras2vec.exe on %s is failed\n", fn);
         exit(0);
         continue;
      }

      FILE *fd = fopen(fn, "r");
      assert(fd);

      int x, y;
      st_table *st_pts = st_init_table(st_numcmp, st_numhash);
      array_t  *arr_pts = NULL;
      char line[1000];
      while ( fgets(line, 1000, fd )) {
         if ( strncmp(line, "POL", 3) == 0 ||
              strncmp(line, "END", 3) == 0)
            continue;
         assert(sscanf(line, "%d %d", &x, &y) == 2);
         int p = index(x, y);
         //if ( !st_is_member(st_pts, p) ) { // we should add all points at this point
         if ( 1 ) {
            if ( !arr_pts ) {
               arr_pts = array_alloc(int, 0);
               array_insert_last(array_t *, arr_return, arr_pts);
            }
            array_insert_last(int, arr_pts, p);
            st_insert(st_pts, (char *)p, (char *)1);
         }
      }

      int count = array_n(arr_pts);
      int *num;
      if ( st_lookup(st_counter, (char *)count, (char **)&num) ) {
         int num1 = (int)num;
         num1 ++;
         if ( num1 > max_count ) {
            max_count = num1;
            num_ctrl_pts = count;
         }
         st_insert(st_counter, (char *)count, (char *)num1);
      } else {
         st_insert(st_counter, (char *)count, (char *)1 );
         if ( max_count == 0 ) {
            max_count ++;
            num_ctrl_pts = count;
         }
      }

      st_free_table(st_pts);
      fclose(fd);

   // clean files
      if ( 1 )
      {
         char fn1[1000];
         char fn2[1000];
         sprintf(fn1, "%s/region_%d.ply", gbl_opts->output_prefix, region->rid);
         sprintf(fn2, "%s/region_%d.tif", gbl_opts->output_prefix, region->rid);
         char command[1000];
         char *del_cmd = NULL;
#ifdef LINUX
         del_cmd = "rm";
#else
         del_cmd = "del";
#endif      
         sprintf(command, "%s %s %s", del_cmd, fn1, fn2);
         char *command_slash = str_replace(command, "/", "\\", 0, -1);
         system(command_slash);
      }
      
   }

   printf("INFO: The total number of control points is %d, %d out of %d are good!\n",
          num_ctrl_pts,
          max_count,
          array_n(arr_return));
   assert( array_n(arr_return) > 0 );
   return arr_return;
}

vector<int> *ksd_taper_compute_converge_from_taper_array(array_t *taper_arr,
                                                         array_t *arr_reg_boundary,
                                                         vector<int> &taper_base,
                                                         vector<int> &taper_top,
                                                         int num_pts,
                                                         int layer_index)
{
   // arr_reg_boundary : the control points of the keyslices
   // num_pts: majority number of control points for each keyslices.

   int k;
   array_t *arr_pts;
   array_t *base_pts = array_fetch(array_t *, arr_reg_boundary, 0);
   st_table *st_pt2arr = st_init_table(st_ptrcmp, st_ptrhash);

   assert(array_n(taper_arr) == array_n(arr_reg_boundary));

   int valid_slice = 1;
   arrayForEachItem(array_t *, arr_reg_boundary, k, arr_pts ) {

      if ( array_n(arr_pts) != num_pts ) {
         if ( k == 0 ) {
            printf("FATAL: the first base keyslice doesn't have the standard %d control points.\n", num_pts);
            exit(0);
         } else {
            printf("INFO: this keyslice #%d doesn't have the standard %d control points.\n", k, num_pts);
         }
         continue;
      }

      if ( k == 0 )
         continue;

      st_table *st_matched = st_init_table(st_numcmp, st_numhash);
         
      bool valid = true;
      // 1st: compute the matching points
      // always use the base keyslice as reference, compute for each base pt, the closest point
      // in the set of control points of new keyslice.
      {
         int j, pt;
         int x, y;
         st_table *st_samePts = st_init_table(st_numcmp, st_numhash);
         arrayForEachItem(int, base_pts, j, pt) {

            if ( st_is_member(st_samePts, (char *)pt) )
               continue;
            else
               st_insert(st_samePts, (char *)pt, (char *) 0);
           
            int x0 = x_axis(pt);
            int y0 = y_axis(pt);
            int k1, new_pt, matched_pt, max_dist = 100000;
            arrayForEachItem(int, arr_pts, k1, new_pt) {
               x = x_axis(new_pt);
               y = y_axis(new_pt);
               if ( abs(x - x0) + abs(y - y0) < max_dist) {
                  max_dist = abs(x-x0) + abs(y-y0);
                  matched_pt = new_pt;
               }
            }

            int *tmp;
            if ( !st_lookup(st_matched, (char *)matched_pt, (char **)&tmp) )
               st_insert(st_matched, (char *)matched_pt, (char *) pt);
            else {
               printf("WARNING: COI point: [%d, %d], matched by both [%d,%d] and [%d,%d]\n", x, y,
                      x0, y0, x_axis((int)tmp), y_axis((int)tmp));
               valid = false;
            }
         }
         st_free_table(st_samePts);
         
      }

      // 2nd: compute the end points at plane layer_index
      {
         if ( valid ) {
            
            valid_slice ++;

            int layer_base = array_fetch(KSD_REGION *, taper_arr, 0)->start;
            int layer_here = array_fetch(KSD_REGION *, taper_arr, k)->start;

            if ( layer_base >= layer_here ) {
               printf("WARNING: BASE: %d, HERE: %d\n", layer_base, layer_here);
               continue;
            }
                  
            assert( layer_base < layer_here);
            assert( layer_here < layer_index );

            // get the points in layer index;
            st_generator *stGenMatched;
            int *here_pt, *base_pt;
            st_foreach_item(st_matched, stGenMatched, (char **)&here_pt, (char **)&base_pt) {
               int x_b = x_axis((int)base_pt);
               int y_b = y_axis((int)base_pt);
               int x_h = x_axis((int)here_pt);
               int y_h = y_axis((int)here_pt);

               // here, the triangle equation is: (x-x1)/(x1-x0) = h1/h2
               float height_ratio = (float)(layer_index - layer_here)/(float)(layer_here - layer_base);
               int x = x_h + ((x_h - x_b)*height_ratio + 0.5);
               int y = y_h + ((y_h - y_b)*height_ratio + 0.5);

               // global hash
               array_t *base_pt_arr;
               if ( !st_lookup(st_pt2arr, (char *)base_pt, (char **)&base_pt_arr) ) {
                  base_pt_arr = array_alloc(int, 0);
                  st_insert(st_pt2arr, (char *)base_pt, (char *)base_pt_arr);
               }
               array_insert_last(int, base_pt_arr, index(x, y));
                  
            }
         }
      }

   }

   // now compute the average point
   st_table *st_pt2pt = st_init_table(st_ptrcmp, st_ptrhash);
   {
      st_generator *stGen;
      int *base_pt;
      array_t *pt_arr;
      st_foreach_item(st_pt2arr, stGen, (char **)&base_pt, (char **)&pt_arr) {

         int index_k;
         int pt, x_total = 0, y_total = 0;
         arrayForEachItem(int, pt_arr, index_k, pt) {
            int x = x_axis(pt);
            int y = y_axis(pt);
            x_total += x;
            y_total += y;
         }

         {
            // get the mid value
            int outlier_thres = 8;
            int total_pts = array_n(pt_arr);
            int x_ave = (int) ((float)x_total/(float)total_pts + .5);
            int y_ave = (int) ((float)y_total/(float)total_pts + .5);

            int index_k, pt;
            int good_num = 0;
            int good_x = 0, good_y  = 0;
            arrayForEachItem(int, pt_arr, index_k, pt) {
               int x = x_axis(pt);
               int y = y_axis(pt);
               if ( abs(x-x_ave) + abs(y-y_ave) < outlier_thres ) {
                  good_num ++;
                  good_x += x;
                  good_y += y;
               } else {
                  printf("INFO: find a outlier point!\n");
               }
            }

            int good_ave_x = (int) ( (float)good_x/(float)good_num + .5);
            int good_ave_y = (int) ( (float)good_y/(float)good_num + .5);
            printf("INFO: %d out of %d are non-outliers (good pts) !!!!\n", good_num, total_pts);
            
            st_insert(st_pt2pt, (char *)base_pt, ( char *) index(good_ave_x, good_ave_y));
         }

      }
   }

   printf("INFO: %d out of %d are valid slices!!!\n", valid_slice, array_n(arr_reg_boundary));

   // adjust the top points by merging the pts close enough to each other
   {
      int same_pt_thres = 2;
      array_t *arr_group = array_alloc(vector<int> *, 0);  // v[i]: base_pt, v[i+1]: top_pt
      st_generator *stGenMatched;
      int *base_pt, *top_pt;
      int j;
      vector<int> *vec_grp;
      st_foreach_item(st_pt2pt, stGenMatched, (char **)&base_pt, (char **)&top_pt) {
         int x = x_axis((int)top_pt);
         int y = y_axis((int)top_pt);
         bool found = false;
         arrayForEachItem(vector<int> *, arr_group, j, vec_grp) {
            int x_ref = x_axis((*vec_grp)[1]);
            int y_ref = y_axis((*vec_grp)[1]);
            if ( abs(x - x_ref) + abs(y - y_ref) <= same_pt_thres) {  // if top_pt is very close to the st_grp[0]
               vec_grp->push_back((int)base_pt);
               vec_grp->push_back((int)top_pt);
               found = true;
            }
         }

         if ( !found ) {
            vec_grp = new vector<int> ();
            vec_grp->push_back((int)base_pt);
            vec_grp->push_back((int)top_pt);
            array_insert_last(vector<int> *, arr_group, vec_grp);
         }
      }

      printf("All %d pair of taper control pts can be divided into %d group(s).\n", st_count(st_pt2pt), array_n(arr_group));
      arrayForEachItem(vector<int> *, arr_group, j, vec_grp) {
         if ( vec_grp->size() > 2 ) {
            int total_x = 0;
            int total_y = 0;
            for (int i = 0; i < vec_grp->size(); i += 2) {
               total_x += x_axis((*vec_grp)[i+1]);
               total_y += y_axis((*vec_grp)[i+1]);
            }
            int x_ave = (int)((float)total_x/(float)(vec_grp->size()/2) + .5);
            int y_ave = (int)((float)total_y/(float)(vec_grp->size()/2) + .5);
            int pt_ave = index(x_ave, y_ave);
            for (int i = 0; i < vec_grp->size(); i += 2) {
               assert(st_is_member(st_pt2pt, (char *)(*vec_grp)[i]));
               st_insert(st_pt2pt, (char *)(*vec_grp)[i], (char *)pt_ave);
            }
            printf("GROUP %d average point: [%d, %d]\n", j, x_ave, y_ave);
         }
      }
   }
   
   // fill up the taper_base and taper_top
   {
      int k;
      int pt;
      arrayForEachItem(int, base_pts, k, pt) {
         taper_base.push_back(pt);
         int *top_pt;
         int ret = st_lookup(st_pt2pt, (char *)pt, (char **)&top_pt);
         assert(ret);
         int tpt = (int)top_pt;
         taper_top.push_back(tpt);
         printf("BASE POINT [%d, %d] -> [%d, %d] TOP POINT!\n", x_axis(pt), y_axis(pt), x_axis(tpt), y_axis(tpt));
      }
   }
   
}

// Function: ksd_taper_compute_converge
//
// Description: compute the converging points for each taper structure
//
// Algorithm:
//    1. For each layer of the taper keyslice, compute the intersection point of the converging
//       point with the plane index.
//
//    2. for all the intersection points $p$ in $P$, compute the average value.
//       This is done by first removing outlier point. the all average point $p_a$ is computed for $(x, y)$,
//       outlier points are those far away from point $p_a$.
//       after removing outliers, compute the average point $\bar(p)$ for the valid points.
//
//    3. check the valid points for all base control points.
//       if two points are very close to each other, merge them.
//       if two points have very closed x/y, but far away y/x, merge x/y.
//
// Post Condition:
//    taper_base: vector<int> storing the base control points.
//    taper_top : vector<int> storing the corresponding top control points of top_base;
// 
void ksd_taper_compute_converge(array_t *taper_arr, vector<int> &taper_base, vector<int> &taper_top, int index)
{
   // compute the convergencing point based on the regions.
   // const int N = array_n(taper_arr);
   const int N = 2;

   int num_ctrl_pts = 0;
   array_t *arr_reg_boundary = ksd_taper_compute_each_region_control_points(taper_arr, num_ctrl_pts, N);
   ksd_taper_compute_converge_from_taper_array(taper_arr,
                                               arr_reg_boundary,
                                               taper_base, taper_top,
                                               num_ctrl_pts,index);
   
}

void ksd_taper_structure_computation(KSD_REGION *last_region, array_t *taper_arr, int index, char *fn)
{
   // first let's check what kind of taper structure it is.
   // type: 1 - taper-to-line, 0 - taper-to-point

   // it seems not necessary needed.
   // int type = ksd_taper_compute_type(last_region, taper_arr, index);

   // second, compute the converging point;
   vector<int> taper_base, taper_top;
   ksd_taper_compute_converge(taper_arr, taper_base, taper_top, index);
   
   // convert the TAPER structure to IR format
   {
      KSD_REGION *region = array_fetch(KSD_REGION *, taper_arr, 0);

      char IR_fn[1000];
      sprintf(IR_fn, "%s/%s", gbl_opts->output_prefix, fn);
      FILE *fd_out = fopen(IR_fn, "at");
      assert(fd_out);

      assert(taper_base.size() == taper_top.size());
      fprintf(fd_out, "BEGIN POLYGON\n");
      for(int i = 0; i < taper_base.size(); i ++ ) {
         int x = x_axis(taper_base[i]);
         int y = y_axis(taper_base[i]);
         fprintf(fd_out, "%d %d\n", x, y);
      }
      fprintf(fd_out, "TAPERED\n");
      for(int i = 0; i < taper_top.size(); i ++ ) {
         int x = x_axis(taper_top[i]);
         int y = y_axis(taper_top[i]);
         fprintf(fd_out, "%d %d\n", x, y);
      }
      fprintf(fd_out, "HEIGHT\n");
      fprintf(fd_out, "%f %f\n", (float)region->start/gbl_opts->total_slices, (float)index/gbl_opts->total_slices);
      fprintf(fd_out, "END POLYGON\n");

      fclose(fd_out);
   }
}

// Function: ksd_with_segmentation_and_tapers
//
// Description: infer the taper structures (to-point|to-line) as well as extrusion
//
// Command:
//     Project.exe -K -O "-L 100 -R 849 -f seg_ir.txt -A 4 -s 1000 -t 0.1
//     -P result/point_cloud_TP_1000/slices_1000_1024_392_bottom_up/image_slice
//     -O result/point_cloud_TP_1000/slices_1000_1024_392_bottom_up_seg_1000_1_1"
//
// Command to generate iv model:
//     project -j -O "-p result\point_cloud_TP_1000\seg_ir.txt
//     -P result\point_cloud_TP_1000\seg_ir.iv
//     -t 7 -w 1024 -M 2 -D 1 -x -4.0 -X 14.0 -y -2.0 -Y 18.0 -z -5.0 -Z 5.0"  or
//     -t 11 -0 1 -1 0 -D 0 -M 2 -w 1024 -x -4.0 -X 14.0 -y -2.0 -Y 18.0 -z -5.0 -Z 5.0"
//     
//
// Algorithm:
//
//   1. generate 2D slices from synthetic 3D model with generate_slices_for_model_tapers()
//
//   2. segment the 2D images, locate the extrusion and taper structures based on keyslice frequencey
//
//   3. for tapered structures, infer the taper to a line and taper to a point respectively.
//
//   4. we should detect tapered structures based on centroid and frequency of keyslices. we can always
//      assume that, for tapered structures, the segments in above/new slices is always SMALLER than under/old
//      slices.
//
// Goal:
//   reconstruct a model which containts both taper-to-point and taper-to-line. The error between
//   the original (ground truth) and the reconstructed one should be measureable.
// 
// Improvement:
//   try a more challenge model where right side of tower and left side of house merged.
//
void ksd_with_segmentation_and_tapers(void **params)
{
   // Assume the 2D synthetic slices have been generated.
   // segment the 2D images to get the locations of taper structures.

   /*
   int step_num = 3;
   int sample_rate = 1000;
   int seg_thres = 4; // doesn't matter
   char *input_path = "result/point_cloud_TP_1000/slices_1000_1024_392_bottom_up";
   char *output_path = NULL;
   */

   // initialize the global vars
   image_width = -1;
   image_height = -1;
   int step_num = 1;
   
   // some thresholds for comparison and matching
   /*
     Here we have 3 thresholds for segementation and keyslices detection.
     To merger all these 3 threshold, the algorithm is based on the observation:
     1 - 0.9 -0.8; 5 - 0.5 - 0.4; 9 - 0.1 - 0.1
     We can choose a unique threshold \tau_d to be [0, 1].
     thres_mask would be (int)(10*\tau_d); => [0, 10];
     thres_region = thres_data = (1 - \tau_d) => [1, 0];  // what does 0 matched mean :P
     we should have a way to set these 3 parameters separately.
     if \tau_d is less than 0, the default (3/0.5/0.4) for the above thresholds are used.
     the SMALLER the \tau_d is, the more ACCURATE the final result will be.
    */
   int   thres_mask   = 3;   /* mask for region */
   float thres_region = 0.5; /* matched region ratio */
   float thres_data   = 0.4; /* matched data ratio */
   //const float thres_region = 0.9; /* matched region ratio */
   //const float thres_data   = 0.8; /* matched data ratio */
   
   // decode the parameters
   bool db = false; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_prefix  = (char *) params[3];
   char *IR_fn = (char *) params[4];
   thres_mask   = (int) params[5];
   thres_region = *((float *) params[6]);
   thres_data   = *((float *) params[7]);
   float tau_d  = *((float *) params[8]);
   int  seg_app = ( int ) params[9];

   if ( tau_d >= 0.0 ) {
      assert( tau_d <= 1.0 );
      thres_mask = (int) ( 10 * tau_d );
      thres_region = 1 - tau_d;
      thres_data = thres_region;
      printf("The thresholds are, thres_mask: %d, thres_region: %f, thres_data: %f\n", thres_mask, thres_region, thres_data);
   }

   assert( input_prefix );
   assert( output_prefix );

   // clean file if exist
   char clean_IR_fn[1000];
   sprintf(clean_IR_fn, "%s/%s", output_prefix, IR_fn);
   FILE *fd_out = fopen(clean_IR_fn, "w");
   assert(fd_out);
   fclose(fd_out);
   
   // start the computation
   int k;
   array_t *gbl_active_regions = array_alloc(KSD_REGION *, 0);
   array_t *arr_log_file = array_alloc(vector<int> *, 0);
   st_table *st_map_A = st_init_table(st_ptrcmp, st_ptrhash);
   st_table *st_map_B = st_init_table(st_ptrcmp, st_ptrhash);
   st_generator *stGen;
   vector<int> *log_region;
   BYTE **new_im = NULL, **ref_im = NULL;
   for (int i = left_num; i < right_num; i += step_num )  // let's do it with interval of step_num
   {
      st_table *st_all_regions = NULL;  // all the region pixels including data
      st_table *st_all_data    = NULL;  // all the data only
      st_table *st_region = NULL;       // for each individual region
      st_table *st_data   = NULL;       // for each data structure.

      // ??? should we check the emptyness of the image ???
      
      char fn[1000];
      char *num_str = digit_string(4, i);
      sprintf(fn, "%s_%s.png", input_prefix, num_str);

      printf("INFO: computing %s\n", fn);
      
      // compute the regions.
      st_table *pixel2rid = st_init_table(st_ptrcmp, st_ptrhash);
      comp_segments(fn, st_all_regions, st_all_data, new_im, pixel2rid, thres_mask, seg_app);

      // check whether this is a blank image.
      /*
      if ( st_count(st_all_regions) == 0 ) {
         free_image(new_im);
         st_free_table(st_all_data);
         st_free_table(st_all_regions);
         st_free_table(pixel2rid);
         continue;
      }
      */

      // draw the segmentation regions for debug
      if ( db ) {
         sprintf(fn, "%s/image_regions_of_slice_%s.png", output_prefix, num_str);
         draw_3D_region_from_hash(fn, st_all_regions, st_all_data);
      }
      
      if ( i == left_num || array_n(gbl_active_regions) == 0 ) {

         // init the active table T.
         printf("Init the reference due to 0 size of gbl_active_regions\n");
         st_foreach_item(st_all_regions, stGen, (char **)(&st_region), (char **)(&st_data)) {
            assert(st_region);
            assert(st_data);
            KSD_REGION *region = new_ksd_region(i);
            hash_2_vector(st_region, region->data_region);
            hash_2_vector(st_data, region->data_structure);
            ksd_compute_centroid_for_region(region);
            array_insert_last(KSD_REGION *, gbl_active_regions, region);
         }

         ref_im = new_im;
         st_free_table(st_all_data);
         st_free_table(st_all_regions);
         st_free_table(pixel2rid);
         continue;
      }

      // draw the active regions for debug
      if ( db ) {
         sprintf(fn, "%s/image_regions_of_slice_%s_active.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, gbl_active_regions);
      }

      // compute centroid for each region in st_all_regions;
      // the result is saved in st_region_centroid<st_region*, centroid>;
      st_table *st_region_centroid = ksd_compute_centroid_all_regions(st_all_regions);
      
      // region-based comparison.
      st_table *st_matched_regions = st_init_table(st_ptrcmp, st_ptrhash);
      array_t *arr_unmatched_active_regions = array_alloc(KSD_REGION *, 0);
      array_t *arr_unmatched_new_regions    = array_alloc(KSD_REGION *, 0);
      array_t *new_active_regions = array_alloc(KSD_REGION *, 0);
      KSD_REGION *region;
      arrayForEachItem(KSD_REGION *, gbl_active_regions, k, region) {
         bool matched_data = false;
         
         // find the matched region
         vector<int> new_region;
         st_table *st_new_region = NULL;
         if ( find_matched_region (region, st_all_regions, new_region, st_new_region, pixel2rid, thres_region) &&
              find_matched_structure(region->data_structure, new_region, ref_im, new_im, thres_data) ) {
               matched_data = true;
         }

         // if !find_matched_region -> no matched data
         // or !find_matched_structure (no matched data)
         if ( !matched_data ) {

            // check whether this region R_1 match with others w.r.t. centroid
            st_table *st_ctrd_matched_region = NULL;
            int is_ctrd_matched = ksd_matched_centroid(region, st_region_centroid, 4, st_ctrd_matched_region);
            if ( is_ctrd_matched ) {
               printf("INFO: at slice %d, region %d was not matched, but has a centroid matched.\n",i, region->rid);
               assert ( st_ctrd_matched_region );
               st_insert(st_map_A, (char *)region, (char *)st_ctrd_matched_region);

               // if this is the start point of taper structure, let's save the extruded part first
               if ( i - region->start > 5 * step_num ) {
                  region->end = i;
                  ksd_generate_polygon(region, output_prefix, IR_fn, db);
                  region->start = i; // reset the region for potential taper structure.
               }
               
               continue;
            } else if ( st_lookup(st_map_B, ( char *)region, (char **)NULL) ) {
               st_insert(st_map_A, (char *)region, (char *)NULL);
               continue;
            }
               
            // dump this region to log file (end of a sub region)
            vector<int> *log_region = new vector<int>();
            region->end = i;
            log_region->push_back(region->rid);
            log_region->push_back(region->start);
            log_region->push_back(region->end);
            array_insert_last(vector<int> *, arr_log_file, log_region);

            // call BPA to generate the boundary
            ksd_generate_polygon(region, output_prefix, IR_fn, db);
            array_insert_last(KSD_REGION *, arr_unmatched_active_regions, region);
            
         } else {
            st_insert(st_matched_regions, (char *)st_new_region, (char *) 1);
            array_insert_last(KSD_REGION *, new_active_regions, region);
         }
      }

      // check the possible TAPER structures in st_map_A & st_map_B
      // and clean up the regions in the st_map_A;
      {
         KSD_REGION *region;
         st_table *st_ctrd_matched_region = NULL;
         st_foreach_item(st_map_A, stGen, (char **)&region, (char **)&st_ctrd_matched_region) {

            // if r is matched by other region or r is larger than its base
            if ( st_ctrd_matched_region == NULL ||
                 st_is_member(st_matched_regions, (char *)st_ctrd_matched_region) ||
                 //ksd_compare_region_size(region, st_ctrd_matched_region) < 0 )
                 region->data_structure.size() - st_count(st_ctrd_matched_region) < 0 )
            {
               // post-processing the region
               // also do taper structure inference???

               array_t *taper_arr = NULL;
               if ( !st_lookup(st_map_B, ( char *)region, (char **)&taper_arr) ||
                    array_n(taper_arr) < 1)
               {

                  // dump region as above
                  // dump this region to log file (end of a sub region)
                  vector<int> *log_region = new vector<int>();
                  region->end = i;
                  log_region->push_back(region->rid);
                  log_region->push_back(region->start);
                  log_region->push_back(region->end);
                  array_insert_last(vector<int> *, arr_log_file, log_region);
                  
                  // call BPA to generate the boundary
                  ksd_generate_polygon(region, output_prefix, IR_fn, db);
                  array_insert_last(KSD_REGION *, arr_unmatched_active_regions, region);

               } else {

                  KSD_REGION *arr_start = array_fetch(KSD_REGION *, taper_arr, 0);
                  KSD_REGION *arr_end   = array_fetch_last(KSD_REGION *, taper_arr );
                  printf("INFO: at slice %d, FOUND a TAPER Structure!!!!\n",i);
                  printf("INFO: TAPER starts: %d, TAPER ends: %d\n",arr_start->start, arr_end->start);
                  ksd_taper_structure_computation(region, taper_arr, i - step_num, IR_fn);
               }
               
            } else {

               // add a new region to matched_region
               st_table *st_r2d = NULL;
               st_lookup(st_all_regions, (char *)st_ctrd_matched_region, (char **)&st_r2d);
               assert(st_r2d);
               KSD_REGION *new_region = new_ksd_region(i);
               hash_2_vector(st_ctrd_matched_region, new_region->data_region); 
               hash_2_vector(st_r2d, new_region->data_structure);
               ksd_compute_centroid_for_region(new_region);
               st_insert(st_matched_regions, (char *)st_ctrd_matched_region, (char *) 1);
               array_insert_last(KSD_REGION *, new_active_regions, new_region);
               array_insert_last(KSD_REGION *, arr_unmatched_new_regions, new_region);

               // remove the region from gbl_active_region; AUTOMATICALLY (not inserted)
               // insert it to the array; update st_map_B with new region
               array_t *taper_arr = NULL;
               if ( !st_lookup(st_map_B, (char *)region, (char **)&taper_arr) ) {
                  taper_arr = array_alloc(st_table *, 1);
                  st_insert(st_map_B, (char *)region, (char *)taper_arr);
               }
               assert(taper_arr);
               array_insert_last(KSD_REGION *, taper_arr, region);
               st_insert(st_map_B, (char *)new_region, (char *)taper_arr);
            }
         } // all centroid matched regions
      }  // empty {}

      // check the new regions of the new image that is not matched by any reference region.
      st_foreach_item(st_all_regions, stGen, (char **)&st_region, (char **)&st_data) {
         if (!st_is_member(st_matched_regions, (char *)st_region)) {
            assert(st_region);
            assert(st_data);
            KSD_REGION *region = new_ksd_region(i);
            hash_2_vector(st_region, region->data_region); 
            hash_2_vector(st_data, region->data_structure);
            ksd_compute_centroid_for_region(region);
            array_insert_last(KSD_REGION *, new_active_regions, region);
            array_insert_last(KSD_REGION *, arr_unmatched_new_regions, region);
         }
      }

      // draw the active regions for debug
      if ( db ) {
         sprintf(fn, "%s/image_regions_of_slice_%s_unmatched_active.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, arr_unmatched_active_regions);
         sprintf(fn, "%s/image_regions_of_slice_%s_unmatched_new_region.png", output_prefix, num_str);
         draw_3D_region_from_array(fn, arr_unmatched_new_regions);
      }
      
      // update ref_im for comparison of next step
      if (ref_im) {
         free_image(ref_im);
         ref_im = NULL;
      }
      array_free(gbl_active_regions);
      gbl_active_regions = new_active_regions;
      array_region_to_image(ref_im, gbl_active_regions);

      // free resource for each iteration
      free(num_str);
      free_image(new_im);
      st_free_table(st_matched_regions);
      st_free_table(pixel2rid);
      array_free(arr_unmatched_new_regions);

      // we should be able to reclaim the memory of regions in arr_unmatched_active_regions.
      KSD_REGION *tmp_region;
      arrayForEachItem(KSD_REGION *, arr_unmatched_active_regions, k, tmp_region) {
         delete tmp_region;
      }
      array_free(arr_unmatched_active_regions);

      // reclamin st_all_region, st_all_data
      st_foreach_item(st_all_regions, stGen, (char **)&st_region, (char **)&st_data) {
         st_free_table(st_region);
         st_free_table(st_data);
      }
      st_free_table(st_all_data);
      st_free_table(st_all_regions);
      st_free_table(st_map_A);
      st_map_A = st_init_table(st_ptrcmp, st_ptrhash);
   }

   // Now we have done all the keyslices computation, let's put the remaining active
   // regions into final report.
   {
      KSD_REGION *region;
      arrayForEachItem(KSD_REGION *, gbl_active_regions, k, region) {
         vector<int> *log_region = new vector<int>();
         region->end = right_num - 1;
         log_region->push_back(region->rid);
         log_region->push_back(region->start);
         log_region->push_back(region->end);
         array_insert_last(vector<int> *, arr_log_file, log_region);
         
         // call BPA to generate the boundary
         ksd_generate_polygon(region, output_prefix, IR_fn, db);
      }
   }

   // print out the final IR based on the log file?
   // This could be done during the processing of finding each individual local region.
   char region_fn[1000];
   sprintf(region_fn, "%s/all_regions.txt", output_prefix );
   FILE *fd = fopen(region_fn, "wt");
   arrayForEachItem(vector<int> *, arr_log_file, k, log_region) {
      fprintf(fd, "%d %d %d\n", log_region->at(0), log_region->at(1), log_region->at(2));
   }
   fclose(fd);

}

void ksd_bpa_boundary_pp( deque<int> *points, BYTE **cur_im)
{

   int len = points->size();

   // try to reshape the windows corner part due to BPA
   // only consider the top and bottom
   // first compute the max and min of Y
   int min_y = image_height, max_y = 0;
   for (int i = 0; i < len; i ++ )
   {
      int y = y_axis((*points)[i]);
      if ( y > max_y )
         max_y = y;
      else if ( y < min_y )
         min_y = y;
   }
   
   for (int i = 0; i < len - 1; i ++ )
   {
      int x0 = x_axis((*points)[i]);
      int y0 = y_axis((*points)[i]);
      int x1 = x_axis((*points)[i+1]);
      int y1 = y_axis((*points)[i+1]);

      // only consider the top and bottom
      if ( x0 == x1 ||
           ( abs(y0 - min_y) > 5 && abs(y0 - max_y) > 5 &&
             ( abs(y1 - min_y) > 5 && abs(y1 - max_y) > 5 )))
         continue;
      
      float slope = abs((float)(y1 - y0)/(float)(x1 - x0));
      if ( slope > 0.2 && slope < 2.0
           && i > 1
           && ! is_good_matched(cur_im, image_height, image_width, x0, y0, x1, y1, 0.2) ) {

         // printf("consider [%d, %d] -> [%d %d], slope: %3f\n", x0, y0, x1, y1, slope);
         int pre_x = x_axis((*points)[i-1]);
         int pre_y = y_axis((*points)[i-1]);
         int pos_x = x_axis((*points)[(i+1)%len]);
         int pos_y = y_axis((*points)[(i+1)%len]);

         int x_extra, y_extra;

         // here we have 8 cases:
         // 1,2: right bottom corner, 1 - hori -> vertical; 2 - vertical -> hori
         // 3,4: left  bottom corner, 4 - hori -> vertical; 3 - vertical -> hori
         // 5,6: right bottom corner, 5 - hori -> vertical; 6 - vertical -> hori
         // 7,8: right bottom corner, 8 - hori -> vertical; 7 - vertical -> hori
         bool isVert2Hori = abs(pre_y - y0) > abs(pre_x - x0);
         if ( isVert2Hori ) {
            x_extra = x0;
            y_extra = y1;
         } else {
            x_extra = x1;
            y_extra = y0;
         }

         /*
         if ( !is_good_matched(cur_im, image_height, image_width, x0, y0, x_extra, y_extra, 0.4) ||
              !is_good_matched(cur_im, image_height, image_width, x1, y1, x_extra, y_extra, 0.4))
            continue;
         */
         
         int new_point = index(x_extra, y_extra);
         points->insert(points->begin() + (i + 1)%len, new_point);
         i ++;
         len ++; // a bug fixed here, increase len by 1.
      }
   }

   // here, we want to simplify the boundary, namely merge the point on the same line
   // the image_width and image_height should be ok for x_axis, y_axis, index, computation.
   if (gbl_opts->BPA_merge_boundary_points) {
      
      int len = points->size() - 2;
      for (int i = 0 ; i < len;  i++)
      {
         int x0 = x_axis((*points)[i]);
         int y0 = y_axis((*points)[i]);
         int x1 = x_axis((*points)[i+1]);
         int y1 = y_axis((*points)[i+1]);
         int x2 = x_axis((*points)[i+2]);
         int y2 = y_axis((*points)[i+2]);
         
         if ( ((x0 == x1) && (x1 == x2)) ||
              ((y0 == y1) && (y1 == y2)) ||
              ((y2 - y1)*(x1 - x0) == (y1 - y0)*(x2 - x1) ) )  // same as (y2-y1)/(x2-x1) == (y1-y0)/(x1-x0)
         {
            points->erase(points->begin() + i + 1);
         
            i --;
            len --;
         }
      
      }
   }
         
}

void get_neighbor_boundary_vec(int pt,
                               vector<int> &vec_mask,
                               int mask,
                               vector<int> &vec_mask_boundary)
{
   assert(image_width > 0 && image_height > 0);
   int x = x_axis(pt);
   int y = y_axis(pt);

   for (int i = y - mask; i <= y + mask; i ++)
      for (int j = x - mask; j <= x + mask; j ++)
         vec_mask.push_back(index(j, i));

   if ( 1 ) {
      for (int i = y - mask; i <= y + mask; i ++)
         for (int j = x - mask; j <= x + mask; j += 2*mask)
            vec_mask_boundary.push_back(index(j, i));
      for (int i = y - mask; i <= y + mask; i += 2*mask)
         for (int j = x - mask + 1; j < x + mask; j ++) // avoid duplicated!!!
            vec_mask_boundary.push_back(index(j, i));
      
   }
}

// helper: check whether there is a region in the image that contains a messy.
bool isThisMajoriyPlaneImage ( BYTE **im, int thres )
{
   // should we do image_dilation to enhance the input? not for now.

   // compute the mask of thres for data point?

   int value = (thres - 1)/2;
   int total_thres = (int) (thres * thres * 0.9);
   for (int y = 0; y < image_height; y ++ )
      for (int x = 0; x < image_width; x ++) {
         if ( im[y][x] == BLACK ) {
            int data_pts = 0;
            for (int j = -value; j <= value; j ++ )
               for (int i = -value; i <= value; i ++ ) {
                  int x0 = x + i;
                  int y0 = y + j;
                  if ( x0 >= 0 && x0 < image_width &&
                       y0 >= 0 && y0 < image_height )
                     data_pts += im[y0][x0] == BLACK ? 1 : 0;
               }
            if ( data_pts > total_thres )
               return true;
         }
      }

   return false;
   
}


// Function: ksd_all_info_fusion
//
// Description: Fuse all information from 3 axis to get the planes and windows for reconstruction.
//
// Command:
//     Project.exe -K -O "-L 0 -R 999 -A 5 -b 50 -D left_right
//     -P result/point_cloud_process_1000/slices_1000_1000_2048_left_right/image_slice
//     -O result/point_cloud_process_1000/slices_0000_ksd_fuse_all"
//
// Command to generate iv model:
//
// Algorithm:
//
//    1. To get the majority planes of the facades from the range data.
//       *. collect the slices with large dark areas where the majority planes reside.
//       *. may integrate couple of those slices to get the whole planes.
//       *. obtain the dark area by CC and remove the outside data points based on CC.
//
//       Todo:
//          mask images to improve the 3 dataset for all directions.
//          
//    2. To get the windows/doors of the facades.
//       *. how to identify the windows accurately?
//       *. keyslice/follow-me on these windows.
//    3. To get other structural parts of the facades.
//       *. how to identify these structures?
//       *. ignore the regions of majority planes and windows.
//       *. extruded on this parts.
//    4. To generate the final model using the above info
//
void ksd_all_info_fusion(void **params)
{
   // decode the parameters
   bool db = false; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_path  = (char *) params[3];
   int thres_plane_region = (int) params[4];
   char *prefix_dir = (char *) params[5];
   int thres_rad = (int ) params[6];

   if ( !prefix_dir ) {
      prefix_dir = "NO_DIR";
   }
   
   // try step 1: get the majority planes of the facades from the range data
   // Project.exe -K -O "-L 200 -R 400 -A 5 -b 30 -D face_inside -P ../slices_1000_1000_2048_left_right/image_slice -O ./"
   if ( 0 )
   {
      char *fn_ext;
      {
         char fn[1000];
         sprintf(fn, "%s_0100.tif", input_prefix);
         if ( !file_exist(fn) ) {
            fn_ext = strdup("png");
         } else {
            fn_ext = strdup("tif");
         }
      }
      
      bool isConsectiveMajorityPlane = false;
      BYTE **plane_im = NULL;
      int  plane_start = 0;
      int  thres = thres_plane_region; // let's reduce the requirment for consective planes.
      for (int i = left_num; i <= right_num; i ++ )
      {
         char *ds = digit_string(4, i);
         char fn[1000];
         sprintf(fn, "%s_%s.%s", input_prefix, ds, fn_ext);
         printf("INFO: processing %s\n", fn);

         BYTE **cur_im = load_image_to_array(fn);
         int len = image_width * image_height;

         if ( isThisMajoriyPlaneImage( cur_im, thres ) ) {
            if ( isConsectiveMajorityPlane ) {
               // integrate majority plane images.
               for (int i = 0; i < len; i++)
                  plane_im[0][i] |= cur_im[0][i];
            } else {
               assert ( !plane_im );
               isConsectiveMajorityPlane = true;
               thres = thres_plane_region / 2;
               plane_start = i;
               plane_im = copy_image(cur_im, image_height, image_width);
            }
         } else {
            if ( isConsectiveMajorityPlane ) {
               isConsectiveMajorityPlane = false;
               thres = thres_plane_region;

               // save the integrated plane image
               char int_fn[1000];
               sprintf(int_fn, "%s/%s_integrated_plane_%d_%d_th_%d.png", output_path, prefix_dir, plane_start, i-1, thres_plane_region);
               printf("PLANE FOUND: %s\n", int_fn);
               Save2File(int_fn, plane_im, NULL, image_width, image_height, 0, NULL);

               // free the resource
               free_image(plane_im);
               plane_im = NULL;
            }
         }

         // free memory resource
         free_image(cur_im);
         free(ds);
      }

      return;
   }

   // enhance p2 by superimpose the boundary pixels b1 to get p3.
   if ( 0 )
   {
      BYTE **im = load_image_to_array("left_right_integrated_plane_808_813_th_30_p2.png");
      vector<int> vec_poly;
      {
         FILE *fd = fopen("global_init_dumped_0.txt", "r");
         assert(fd);
         char str_line[100];
         int x, y;
         while (fgets(str_line, 100, fd)) {
            if ( str_line[0] == 'B' || str_line[0] == 'E' )
               continue;
            sscanf(str_line, "%d %d", &x, &y);
            vec_poly.push_back(index(x, y));
         }
         fclose(fd);
      }

      // draw image based on boundary
      {
         for (int i = 1; i < vec_poly.size(); i ++ ) {
            int x1 = x_axis(vec_poly[i-1]);
            int y1 = y_axis(vec_poly[i-1]);
            int x2 = x_axis(vec_poly[i]);
            int y2 = y_axis(vec_poly[i]);
            draw_line_on_2D_image(im, image_height, image_width, x1, y1, x2, y2);
         }
      }
      Save2File("left_right_integrated_plane_808_813_th_30_p3.png", im, NULL, image_width, image_height, 0, NULL);
      free_image(im);
   }

   // compute the windows/doors for the p3
   // first to find the location of the windows inside boundary b1. and then use BPA to compute the boundary of them
   // project.exe -K -O "-A 5 -P ./ -O ./ -D image_fn.png -b <MASK OF WINDOWS> -r <RAD for BPA inside window> -L <HEIGHT_LO> -R <HEIGHT_HI>"
   {
      printf("Infering windows/doors...\n");
      BYTE **cur_im = load_image_to_array(prefix_dir);
      
      // first compute the valid pixel which are inside the boundary b1
      vector<int> vec_fill;
      {
         vector<int> vec_poly;
         {
            FILE *fd = fopen("global_init_dumped_0.txt", "r");
            FILE *fd_ir = fopen("test_ir.txt", "w");
            assert(fd);
            assert(fd_ir);
            char str_line[100];
            int x, y;
            while (fgets(str_line, 100, fd)) {
               // fprintf(fd_ir, "%s", str_line); // this should be an option
               if ( str_line[0] == 'B' || str_line[0] == 'E' )
                  continue;
               sscanf(str_line, "%d %d", &x, &y);
               vec_poly.push_back(index(x, y));
            }
            fclose(fd);
            fclose(fd_ir);
         }

         fill_polygon(vec_poly, vec_fill);
      }

      // now locate the windows/doors use a relative big mask, say 64x64 (a generic threshold)
      // let's compute all the potential windows/doors in one run
      {
         BYTE **fil_im = new_image(image_height, image_width);
         for (int i = 0; i < vec_fill.size(); i ++)
            fil_im[0][vec_fill[i]] = BLACK;

         if ( db ) {
            Save2File("filled_image.png", fil_im, NULL, image_width, image_height, 0, NULL);
         }
         
         // find a seed based on mask (64x64), and then watershed the seed with smaller mask, say(8x8).
         // until all the window/door is covered, and then find another one.
         bool need_check = true;
         int num_pixel = image_width * image_height;
         int mask_locate = thres_plane_region == 5 ? 12: thres_plane_region; // 16: 64
         int mask_water = mask_locate / 2;
         int num_wd = 0;
         while ( need_check ) {
            need_check = false;
            int x, y;
            for ( int i = 0; i < vec_fill.size(); i ++ )
            {
               // only need to check white pixels for windows/doors
               if ( cur_im[0][vec_fill[i]] == WHITE ) {
                  x = x_axis(vec_fill[i]);
                  y = y_axis(vec_fill[i]);
                  bool isGood = true;
                  for ( int j = y - mask_locate; j < y + mask_locate; j ++ )
                  {
                     for ( int k = x - mask_locate; k < x + mask_locate; k ++ )
                        if ( cur_im[j][k] == BLACK ) {
                           isGood = false;
                           break;
                        }

                     if ( !isGood )
                        break;
                  }
                  if ( isGood ) {
                     need_check = true;
                     break;
                  }
               }
            }

            // found a good seed? -- let's fill the window/door and find another one
            // do not use this code, use the new boundary to fill the window/door region as the following.
            if ( 0 && need_check ) {

               BYTE **new_im = copy_image(cur_im, image_height, image_width);
               
               // watershed the window/door starting at seed [x,y]
               vector<int> vec_outer_pts;
               vector<int> vec_outer_new;
               vec_outer_pts.push_back(index(x, y));
               while ( vec_outer_pts.size() > 0 )
               {
                  for ( int vi = 0; vi < vec_outer_pts.size(); vi ++ ) {
                     vector<int> vec_mask, vec_mask_boundary;
                     get_neighbor_boundary_vec(vec_outer_pts[vi], vec_mask, mask_water, vec_mask_boundary);
                     vector<int> *vec_res = find(cur_im[0], &vec_mask, BLACK);
                     if ( vec_res->size() == 0 ) {
                        vector<int> *vec_new = find(new_im[0], &vec_mask_boundary, WHITE);
                        if ( vec_new->size() > 0 ) {
                           for (int i = 0; i < vec_new->size(); i ++ )
                              vec_outer_new.push_back((*vec_new)[i]);
                           delete vec_new;
                           for (int i = 0; i < vec_mask.size(); i ++)
                              new_im[0][vec_mask[i]] = BLACK;
                        }
                     }
                     delete vec_res;
                  }

                  vec_outer_pts.clear();
                  vec_outer_pts = vec_outer_new;
                  vec_outer_new.clear();
               }

               // save the update image
               printf("Found window No. %d!\n", ++num_wd);
               {
                  char fn[100];
                  sprintf(fn, "debug_window_%d.png", num_wd);
                  Save2File(fn, new_im, NULL, image_width, image_height, 0, NULL);
                  free_image(cur_im);
                  cur_im = new_im;
               }
            }

            // apply BPA inside the window/door to get the boundary
            if ( need_check ) {

               // let's assume the bottom of the window is flattened, this simplifies the p1
               // the center point is (x, y)

               int rad = thres_rad == 0 ? 2 : thres_rad;
               deque<int> *points = new deque<int>;
               POINTS_AUX *points_aux = new POINTS_AUX;

               while ( true ) {
                  if ( cur_im[++y][x] == BLACK )
                     break;
               }
               assert(y < image_height);
               int p1 = index(x, y - rad);
               points->push_back(index(x, y));
               points_aux->end_points.push_back(p1);
               points_aux->radius.push_back(rad);
               points_aux->colors.push_back(0);

               // points is the pivot point, p1 is the ending points.
               BYTE **debug_im = NULL;
               gbl_opts->BPA_debug_image_interval = 20;
               gbl_opts->BPA_debug_mode = 0;
               // call BPA_boundary
               ball_pivoting(cur_im, *points, rad, p1, debug_im, 1, points_aux);

               // post-process the boundary
               gbl_opts->BPA_merge_boundary_points = 1; // adjustable
               ksd_bpa_boundary_pp( points, cur_im );

               BYTE **boundary_im = new_image(image_height, image_width);
               vector<int> win_vec;
               FILE *ir_fd = fopen("test_ir.txt", "a+");
               assert(ir_fd);
               fprintf(ir_fd, "BEGIN POLYGON\n%d %d\n", x_axis((*points)[0]), y_axis((*points)[0]));
               for (int i = 0 ; i < points->size()-1; i ++) {
                  draw_line_on_2D_image(boundary_im, image_height, image_width,
                                        x_axis((*points)[i]),
                                        y_axis((*points)[i]),
                                        x_axis((*points)[i+1]),
                                        y_axis((*points)[i+1]));
                  win_vec.push_back((*points)[i]);
                  fprintf(ir_fd, "%d %d\n", x_axis((*points)[i+1]), y_axis((*points)[i+1]));
               }
               fprintf(ir_fd, "EXTRUSION\nHEIGHT\n%f %f\n",left_num/1000.0, right_num/1000.0);
               fprintf(ir_fd, "END POLYGON\n");
               fclose(ir_fd);
               win_vec.push_back((*points)[0]);

               // let's fill the polygon
               vector<int>  win_vec_fill;
               fill_polygon(win_vec, win_vec_fill);
               for (int i = 0; i < win_vec_fill.size(); i ++)
                  cur_im[0][win_vec_fill[i]] = BLACK;
               
               {
                  char fn[100];
                  static int num_wd = 1;
                  printf("writing windows No. %d\n", num_wd);
                  // sprintf(fn, "debug_window_debug_%3d.png", num_wd);
                  // Save2File(fn, cur_im, NULL, image_width, image_height, 0, NULL);
                  //sprintf(fn, "debug_window_boundary_%d.png", num_wd++);
                  //Save2File(fn, boundary_im, NULL, image_width, image_height, 0, NULL);
                  num_wd ++;
                  free_image(boundary_im);
               }

            }
            
         } // end of while
      }
      
   }
}

bool ksd_find_a_matching_pts(array_t *end_points_new,
                             vector<float> *pts_new,
                             vector<float> &vec_match_params,
                             vector<float> &vec_match_result,
                             array_t *z_arr,
                             int dir,
                             int dir_group,
                             int &global_sign)
{
   const float dist_thres = 2.0;  // This is a hard problem
      
   if ( array_n(end_points_new) == 0 )
      return false;

   int pre_matched_bnd_idx = -1;
   int pre_matched_pts_idx = -1;
   int matched_bnd_idx = -1;
   int matched_pts_idx = -1;

   if ( vec_match_params.size() > 0 ) {
      pre_matched_bnd_idx = (int) vec_match_params[0];
      pre_matched_pts_idx = (int) vec_match_params[1];
   }

   float x = pts_new->at(0);
   float y = pts_new->at(1);
   float z = pts_new->at(2);
   
   int i_outer, i_inner;
   array_t *bnd_pts;
   vector<float> *points;
   float min_dist = 10000.0;
   arrayForEachItem(array_t *, end_points_new, i_outer, bnd_pts) {

      // let's ignore the matching from the same facades.
      int dg_new = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(4);
      if ( dg_new == dir_group ) 
         continue;
      
      int dir_new = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(0);
      if ( dir !=  dir_new )
         continue;

      arrayForEachItem(vector<float> *, bnd_pts, i_inner, points) {
         //float dist = abs(x - (*points)[0]) + abs( y - (*points)[1]) + abs( z - (*points)[2] );
         float dist = sqrt(abs(x - (*points)[0])*abs(x - (*points)[0]) +
                           abs(y - (*points)[1])*abs(y - (*points)[1]) +
                           abs(z - (*points)[2])*abs(z - (*points)[2]));
         if ( dist > dist_thres )
            continue;
         else {
            if ( dist < min_dist ) {
               min_dist = dist;
               matched_bnd_idx = i_outer;
               matched_pts_idx = i_inner;
            }
         }
      }
   }

   // These two elements are counted as 0,1, do NOT move, or move it ACCORDINGLY
   if ( matched_bnd_idx > -1 ) {
      vec_match_result.push_back( matched_bnd_idx );
      vec_match_result.push_back( matched_pts_idx );
   } else {
      if ( i_outer == 8 )
         printf("INFO: NOOOOOOOOO match for point [%f, %f, %f]\n", x, y, z);
   }

   // to avoid duplicated point or missed point in the matced point
   if ( pre_matched_bnd_idx == matched_bnd_idx ) {

      int offset = matched_pts_idx - pre_matched_pts_idx;
      int sign   = offset > 0 ? 1 : offset < 0 ? -1 : 0;
      if ( offset != 0 ) {

         if ( global_sign != -2 ) {
            assert(global_sign == 1 || global_sign == -1 );
            if ( global_sign != sign ) {
               printf("WARNING: may be a fluctuation here!!!!\n");
            }
         } else {
            global_sign = sign;
         }
         
         if ( abs(offset) < 5 && global_sign == sign ) {
            array_t *match_arr = array_fetch(array_t *, end_points_new, matched_bnd_idx);
            //printf("PRE: %d, NOW: %d, SIGN: %d, TOTAL: %d\n", pre_matched_pts_idx, matched_pts_idx, sign, array_n(match_arr));
            for ( int i = pre_matched_pts_idx + sign; i != matched_pts_idx + sign; i += sign ) {
               vector<float> *pts_matched = array_fetch(vector<float> *, match_arr, i);
               vec_match_result.push_back( pts_matched->at(0) );
               vec_match_result.push_back( pts_matched->at(1) );
               vec_match_result.push_back( pts_matched->at(2) );
            }
         } else {
            printf("WARNING: matched pt is far away from previous matched points!!!! - [offset - %d]\n", abs(offset));
         }
      }

      // what is a middle point is being skipped? i.e. offset == 2?
   }

   return vec_match_result.size() > 0;
   
}

bool ksd_find_a_matching_pts_from_other_dirs(array_t *end_points_new,
                                             array_t *end_points,
                                             vector<float> *pts_new,
                                             vector<float> &vec_match_params,
                                             vector<float> &vec_match_result,
                                             array_t *z_arr,
                                             int dir,
                                             int dir_group,
                                             int &global_sign,
                                             int i_inner1,
                                             int i_outer1,
                                             vector<float> &vec_major_coord,
                                             vector<float> &vec_major_slope,
                                             vector<int> &vec_region_indx)
{
   const float dist_thres = 2.0;  // This is a hard problem

   if ( array_n(end_points_new) == 0 )
      return false;

   int pre_matched_bnd_idx = -1;
   int pre_matched_pts_idx = -1;
   int matched_bnd_idx = -1;
   int matched_pts_idx = -1;
   static int first_time_matched_index = -1;
   static int consective_matched_start = -1;
   
   if ( vec_match_params.size() > 0 ) {
      pre_matched_bnd_idx = (int) vec_match_params[0];
      pre_matched_pts_idx = (int) vec_match_params[1];
   }

   float x = pts_new->at(0);
   float y = pts_new->at(1);
   float z = pts_new->at(2);
   
   int i_outer, i_inner;
   array_t *bnd_pts;
   vector<float> *points;
   float min_dist = 10000.0;
   arrayForEachItem(array_t *, end_points_new, i_outer, bnd_pts) {

      // let's ignore the matching from the same facades.
      int dg_new = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(4);
      if ( dg_new == dir_group ) 
         continue;
      
      int dir_new = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(0);
      if ( dir ==  dir_new )
         continue;

      arrayForEachItem(vector<float> *, bnd_pts, i_inner, points) {
         // float dist = abs(x - (*points)[0]) + abs( y - (*points)[1]) + abs( z - (*points)[2] );
         float dist = sqrt(abs(x - (*points)[0])*abs(x - (*points)[0]) +
                           abs(y - (*points)[1])*abs(y - (*points)[1]) +
                           abs(z - (*points)[2])*abs(z - (*points)[2]));
         if ( dist > dist_thres )
            continue;
         else {
            if ( dist < min_dist ) {
               min_dist = dist;
               matched_bnd_idx = i_outer;
               matched_pts_idx = i_inner;
            }
         }
      }
   }

   int slope_coord_a = -1;
   int slope_coord_b = -1;
   if ( dir == 0 ) {
      slope_coord_a = 0; // x
      slope_coord_b = 2; // z
   }
   else if ( dir == 1 ) {
      slope_coord_a = 0; // x
      slope_coord_b = 1; // y
   }
   else if ( dir == 2 ) {
      slope_coord_a = 2; // z
      slope_coord_b = 1; // y
   }
   assert( slope_coord_a > -1 && slope_coord_b > -1 );
   
   // These two elements are counted as 0,1, do NOT move, or move it ACCORDINGLY
   if ( matched_bnd_idx > -1 ) {

      if ( pre_matched_bnd_idx == -1 ) {
         printf("INFO: the region [%d] matched with [%d], start at point index: [%d]\n", i_outer1, matched_bnd_idx, i_inner1);

         first_time_matched_index = matched_pts_idx;
         consective_matched_start = i_inner1;
      }
      
      vec_match_result.push_back( matched_bnd_idx );
      vec_match_result.push_back( matched_pts_idx );

      // check the majority coordinate value
      array_t *match_arr = array_fetch(array_t *, end_points_new, matched_bnd_idx);
      vector<float> *pt =  array_fetch(vector<float> *, match_arr, matched_pts_idx);
      int coord_index = dir == 0 ? 1 : dir == 2 ? 0 : 2;  // 0: y, 1: z, 2: x;
      float coord_value = pt->at(coord_index);

      bool found = false;
      for (int i = 0; i < vec_major_coord.size(); i +=2 ) {
         if ( coord_value == vec_major_coord[i] ) {
            found = true;
            vec_major_coord[ i+1 ] += 1.0;
            break;
         }
      }

      if ( ! found ) {
         vec_major_coord.push_back(coord_value);
         vec_major_coord.push_back(1.0);
      }

      // check the major slope value based on a region
      if ( i_inner1 > 0 ) {
         array_t *cur_arr = array_fetch(array_t *, end_points, i_outer1);
         int data_len = array_n(cur_arr);
         vector<float> *pt_pre =  array_fetch(vector<float> *, cur_arr, (i_inner1 - 1) % data_len );
         vector<float> *points =  array_fetch(vector<float> *, cur_arr, i_inner1 % data_len);
         float diff = abs( points->at(slope_coord_b) - pt_pre->at(slope_coord_b));
         float cur_slope;
         if ( diff == 0 )
            cur_slope = 10000.0;
         else
            cur_slope = abs( points->at(slope_coord_a) - pt_pre->at(slope_coord_a) ) / diff;

         // printf("POINT %d slope is %f\n", i_inner1, cur_slope);
         // update the major slope vector
         bool found = false;
         for (int i = 0; i < vec_major_slope.size(); i +=2 ) {
            if ( cur_slope >= vec_major_slope[i] - 0.2 && cur_slope <= vec_major_slope[i] + 0.2  ) {
               found = true;
               vec_major_slope[ i+1 ] += 1.0;
               break;
            }
         }
         
         if ( ! found ) {
            vec_major_slope.push_back(cur_slope);
            vec_major_slope.push_back(1.0);
         }
      }
      
         
   } else {
      
      if ( pre_matched_bnd_idx != -1 ) {

         // assume the region has started
         printf("INFO: the region [%d] previously matched with [%d], ends at point index: [%d]\n", i_outer1, pre_matched_bnd_idx, i_inner1 -1);
         if ( i_inner1 - consective_matched_start < 10 ) {
            consective_matched_start = -1;
            vec_major_slope.clear();
            return false;
         }

         // compute the major slope based on vec_major_slope;
         printf("The slopes are:");
         int max_index_num = 0, max_index;
         for (int i = 0; i < vec_major_slope.size(); i += 2) {
            printf(" %f[#:%d],", vec_major_slope[i], (int)vec_major_slope[i+1]);
            if ( vec_major_slope[i+1] > max_index_num ) {
               max_index_num = vec_major_slope[i+1]; 
               max_index = i;
            }
         }
         printf(". The maxinum slope is %f, with %d\n", vec_major_slope[max_index], (int) vec_major_slope[max_index+1]);
         
         // now let's update the start and end points in vec_region_indx.size() - 2, .size() - 1
         float major_slope = vec_major_slope[max_index];
         array_t *new_bnd = array_fetch(array_t *, end_points, i_outer1);
         int data_len = array_n(new_bnd);
         int search_start = consective_matched_start;
         int search_end   = i_inner1-1;

         assert( search_end > search_start );
         assert( search_start > 0);
         
         int cur_pos = search_start;
         int segment_start_loc = -1, segment_end_loc = -1;
         int total_num_thres = 5;
         bool isGoodSlope = false;
         int accumulated_bad = 0;
         int accumulated_good = 0;
         printf("INFO: Searching from %d to %d.\n", search_start, search_end);
         while ( cur_pos < search_end + 1 ) {

            // compute the slope for the new point
            vector<float> *pt1 = array_fetch(vector<float> *, new_bnd, (cur_pos) % data_len);
            vector<float> *pt0 = array_fetch(vector<float> *, new_bnd, (cur_pos-1) % data_len);
            float diff = abs( pt1->at(slope_coord_b) - pt0->at(slope_coord_b));
            float cur_slope;
            if ( diff == 0 )
               cur_slope = 10000.0;
            else
               cur_slope = abs( pt1->at(slope_coord_a) - pt0->at(slope_coord_a) ) / diff;

            bool isCurGoodSlope = cur_slope >= major_slope - 0.2 && cur_slope <= major_slope + 0.2 ;

            if ( 0 && cur_pos > 840 )
            printf("=== i_outer1: %d, cur_pos: %d, major_slope: %f, cur_slope:%f, isCurGoodSlope: %d, isGoodSlope: %d\n",
                   i_outer1, cur_pos, major_slope, cur_slope, isCurGoodSlope, isGoodSlope);

            // if the beginning is right slope
            if ( (cur_pos == search_start && isCurGoodSlope) ||
                 (cur_pos == search_end ))
            {
               // extend the point
               int search_loc = -1;
               
               // int search_loc = search_for_the_extended_position(cur_pos == search_start);
               {
                  int sign = cur_pos == search_start ? -1 : 1;
                  int cur_pos1 = cur_pos;
                  while ( true ) {
                     assert(cur_pos1 > -1 && cur_pos1 < array_n(new_bnd));
                     vector<float> *pt1 = array_fetch(vector<float> *, new_bnd, (cur_pos1) % data_len);
                     vector<float> *pt0 = array_fetch(vector<float> *, new_bnd, (cur_pos1+sign) % data_len);
                     float diff = abs( pt1->at(slope_coord_b) - pt0->at(slope_coord_b));
                     if ( diff == 0 )
                        cur_slope = 10000.0;
                     else
                        cur_slope = abs( pt1->at(slope_coord_a) - pt0->at(slope_coord_a) ) / diff;
                     bool isCurGoodSlope = cur_slope >= major_slope - 0.2 && cur_slope <= major_slope + 0.2 ;
                     if ( ! isCurGoodSlope ) {
                        search_loc = cur_pos1;
                        break;
                     } else {
                        cur_pos1 = cur_pos1 + sign;
                     }
                  }
               }

               assert( search_loc > -1 );
               if ( cur_pos == search_start ) {
                  segment_start_loc = search_loc;
                  printf(" INFO: segment_start_loc is %d\n", search_loc);
               } else {
                  printf(" INFO: segment_end_loc is %d\n", search_loc);
                  segment_end_loc = search_loc;
                  break;
               }
            }

            // if from bad -> good,
            if ( cur_pos != search_start && !isGoodSlope && isCurGoodSlope ) {
               segment_start_loc = cur_pos - 1; // starting from previous one.
               printf(" INFO: FROM BAD TO GOOD, so starting AT %d\n", segment_start_loc);
               isGoodSlope = true;
            }

            // if from good -> bad,
            if ( cur_pos != search_start && isGoodSlope && !isCurGoodSlope && accumulated_bad >= total_num_thres ) {
               segment_end_loc = cur_pos - total_num_thres -1; // found the end point, quit
               printf(" INFO: FROM good to bad, so ending AT %d\n", segment_end_loc);
               break;
            } else {
               accumulated_bad ++;
            }

            if ( isCurGoodSlope ) {
               accumulated_bad = 0;
            }
            
            if ( cur_pos == search_start ) {
               printf("INFO: The starting point is %s\n", isCurGoodSlope ? "TRUE" : "FALSE");
               isGoodSlope = isCurGoodSlope;
            }
               
            cur_pos ++;
            
         } // end of while

         printf("The start pos is %d, ends at %d\n", segment_start_loc, segment_end_loc);

         // clear the old dataset
         vec_major_slope.clear();

         // update the region info
         assert( segment_end_loc > -1 && segment_start_loc > -1 && segment_end_loc > segment_start_loc );
         assert( pre_matched_bnd_idx > -1 );
         assert( pre_matched_pts_idx > -1 );
         assert( first_time_matched_index > -1 );
         
         vec_region_indx.push_back(segment_start_loc);
         vec_region_indx.push_back(segment_end_loc);
         vec_region_indx.push_back(i_outer1);
         vec_region_indx.push_back(pre_matched_bnd_idx);
         vec_region_indx.push_back(pre_matched_pts_idx - first_time_matched_index > 0 ? 1 : -1);

         first_time_matched_index = -1; // not necessary to do this.

      }
      
   }

   return vec_match_result.size() > 0;
   
}

// Function: ksd_zipper_boundary
//
// Description: Zipper all facades from all directions
//
// Command:
//     Project.exe -K -O "-A 6 -P IR_all.txt -O ./IR_update_all.txt"
//
// Algorithm:
//
//    The key idea here is to match a point with its closest point less than a threshold.
//    The prerequistion is that for each face, the BPA refinement radius is the same.
//    Therefore for a relative straight line, we can match from new boundary to existed (higher confidence)
//    boundary point by point.
//
//    If two consective points matched the same point, we should ignore one of it. If two
//    consective points matched two separated points, we should add a point.
//
//    If two faces come from the same direction (X, Y or Z), we can change the value of height
//    to match two faces. (This is espeically useful for roof and non-roof part merge)
//
//    If the new boundary has extra parts, say in straight line, we
//    can use mininum squre error method to transform the whole new boundary to existed boundary
//    to the extend of the best matching.
//
//    Here, it is necessary to draw a picture to illustrate the idea.
//
void ksd_zipper_boundarya_app_match(void **params)
{
   // decode the parameters
   bool db = false; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_path  = (char *) params[3];
   int thres_plane_region = (int) params[4];
   char *prefix_dir = (char *) params[5];

   if ( !prefix_dir ) {
      prefix_dir = "NO_DIR";
   }

   // load the IR file
   array_t *z_arr = array_alloc(vector<float> *, 0);
   array_t *end_points = load_end_points_from_IR_with_dirs(input_prefix, z_arr);
   array_t *end_points_new = array_alloc(array_t *, 0);
   array_t *arr_matched_bnds = array_alloc(array_t *, 0);

   // process the matching points one-by-one
   // ksd_update_pts_for_zippering(end_points, z_arr);
   // There are couple of cases for matching
   // 1. the template is REAL extrusion, but the new bnd is FALSE.
   // 2. the template is REAL extrusion, and the new bnd is FALSE.
   // 3. the template is FALSE extrusion, same to the new bnd.
   // 4. the template is FALSE extrusion, but the new bnd is REAL.
   if ( 1 ) 
   {
      int *st_match_rank = (int *)malloc(array_n(end_points) * sizeof(int));

      int i_outer;
      array_t *bnd_pts;
      arrayForEachItem(array_t *, end_points, i_outer, bnd_pts) {

         memset(st_match_rank, 0, array_n(end_points)*sizeof(int));

         // if the first boundary, add them into end_points_new
         // this is automatically done in the ksd_find_a_matching_pts()

         // printf("Checking boundary #%d...\n", i_outer);

         // for the general case
         int dir       = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(0);
         int dir_group = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(4);
         int i_inner1;
         vector<float> *points;
         bool has_found_match = false;
         array_t *boundary_new = array_alloc(vector<float> *, 0);
         int matched_bnd_idx = -1, matched_pts_idx = -1;
         vector<float> vec_match_result;
         vector<float> vec_match_params;
         int global_sign = -2;
         arrayForEachItem(vector<float> *, bnd_pts, i_inner1, points) {

            // here, points contain two extruded points!!!
            
            // check for each point, its corresponding points
            has_found_match = ksd_find_a_matching_pts(end_points_new, points, vec_match_params, vec_match_result, z_arr, dir, dir_group, global_sign);
            if ( vec_match_params.size() > 0 )
               vec_match_params.clear();
            
            if ( ! has_found_match ) {
               vector<float> *cpy_pt =  new vector<float> ( *points );
               array_insert_last(vector<float> *, boundary_new, cpy_pt);
               continue;
            }

            // found a matched point
            // we should consider the case where a large portion of the boundary is matched.
            // so we should not have duplicated points.
            // the return info of the matching should contain the index (r1) for boundary in array, b1,
            // the index (r2) for the point in b1,
            matched_bnd_idx = (int)vec_match_result[0];
            matched_pts_idx = (int)vec_match_result[1];

            if ( vec_match_result.size() > 2 ) {
               for (int i = 2; i < vec_match_result.size(); i += 3) {
                  vector<float> *cpy_pt = new vector<float> ();
                  cpy_pt->push_back(vec_match_result[i]);
                  cpy_pt->push_back(vec_match_result[i+1]);
                  cpy_pt->push_back(vec_match_result[i+2]);
                  array_insert_last(vector<float> *, boundary_new, cpy_pt);
               }
            }

            st_match_rank[matched_bnd_idx] ++;

            vec_match_params.push_back(matched_bnd_idx);
            vec_match_params.push_back(matched_pts_idx);
            vec_match_result.clear();
         }

         array_insert_last(array_t *, end_points_new, boundary_new);

         // if most of points of a boundary is matched, we may want to update the height info to match
         // the previous one if have the same direction.
         {
            int max_num = -1, max_index = -1;

            for (int i = 0; i < array_n(end_points); i ++ ) {
               if ( st_match_rank[i] > max_num ) {
                  max_num = st_match_rank[i];
                  max_index = i;
               }
            }

            if ( max_num > 4 ) {
               
               // check whether both are extruded to the same direction
               vector<float> *vec_info1 = array_fetch(vector<float> *, z_arr, max_index);
               vector<float> *vec_info2 = array_fetch(vector<float> *, z_arr, i_outer);
               int dir1 = (int)(*vec_info1)[0];
               int dir2 = (int)(*vec_info2)[0];
               if ( dir1 == dir2 ) {
                  float base = (*vec_info1)[2];
                  float top  = (*vec_info1)[3];

                  printf("Update the height info: [#%d -> #%d] base from [%f --> %f]\n",
                         max_index, i_outer, vec_info2->at(2), base);
                  vec_info2->at(2) = base;
               }
            }
         }
      } // end of for all end_points


      // second round - zipper face in different faces;
      // 2.1 - transform the facades into the same plane for walls, and collect info for next step
      if ( 1 )
      {
         // free old end_points

         printf("\n=================== 2nd round for different direcionts. =============\n\n");
         end_points = end_points_new;
         end_points_new = array_alloc(array_t *, 0);

         arrayForEachItem(array_t *, end_points, i_outer, bnd_pts) {

            // translate the data by a CONSTANT value;
            if (1)
            {
               const int LEN = 100;
               int array_len = array_n(bnd_pts);
               array_t *dup_arr = array_dup(bnd_pts);
               while ( array_n(bnd_pts) > 0 ) {
                  array_remove_last(bnd_pts);
               }
               for (int i = 0; i < array_len; i ++ ) // the last is duplicated with the first one.
               {
                  if ( (i + LEN) % array_len == 0 )
                     continue;
                  array_insert_last(vector<float> *, bnd_pts, array_fetch(vector<float> *, dup_arr, (i + LEN) % array_len) );
               }
            }
            // end of translation
            
         }
         
         arrayForEachItem(array_t *, end_points, i_outer, bnd_pts) {

            memset(st_match_rank, 0, array_n(end_points)*sizeof(int));

            // if the first boundary, add them into end_points_new
            // this is automatically done in the ksd_find_a_matching_pts()

            // for the general case
            int dir       = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(0);
            int dir_group = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(4);
            int i_inner1;
            vector<float> *points;
            bool has_found_match = false;
            array_t *boundary_new = array_alloc(vector<float> *, 0);
            int matched_bnd_idx = -1, matched_pts_idx = -1;
            vector<float> vec_match_result;
            vector<float> vec_match_params;
            int global_sign = -2;
            vector<float> vec_major_coord;
            vector<float> vec_major_slope;
            vector<int>   vec_region_indx;
            
            arrayForEachItem(vector<float> *, bnd_pts, i_inner1, points) {

               // i_inner1 = i_inner1 + 10;  // maybe we should find out what 10 is by checking machingability.
               
               // here, points contain two extruded points!!!
            
               // check for each point, its corresponding points
               // collect two major info for merging:
               // 1. the majority value of the X,Y,Z for the extrusion part of the new boundary.
               // 2. the majority slope of the matched points.
               has_found_match = ksd_find_a_matching_pts_from_other_dirs(end_points_new, end_points, points, vec_match_params, vec_match_result,
                                                                         z_arr, dir, dir_group, global_sign, i_inner1, i_outer,
                                                                         vec_major_coord, vec_major_slope, vec_region_indx);
               if ( vec_match_params.size() > 0 )
                  vec_match_params.clear();

               // update matched region info
               if ( vec_region_indx.size() > 0 ) {
                  vector<int> *vec_region_info = new vector<int> (vec_region_indx);
                  array_insert_last(vector<int> *, arr_matched_bnds, vec_region_info);
                  vec_region_indx.clear();
               }

               if ( 1 || ! has_found_match ) 
               {
                  vector<float> *cpy_pt =  new vector<float> ( *points );
                  array_insert_last(vector<float> *, boundary_new, cpy_pt);
               }

               // found a matched point
               // we should consider the case where a large portion of the boundary is matched.
               // so we should not have duplicated points.
               // the return info of the matching should contain the index (r1) for boundary in array, b1,
               // the index (r2) for the point in b1,
               if ( has_found_match ) {
                  
                  matched_bnd_idx = (int)vec_match_result[0];
                  matched_pts_idx = (int)vec_match_result[1];

                  st_match_rank[matched_bnd_idx] ++;
                  vec_match_params.push_back(matched_bnd_idx);
                  vec_match_params.push_back(matched_pts_idx);
               }
               
               vec_match_result.clear();
            }

            array_insert_last(array_t *, end_points_new, boundary_new);

            // if most of points of a boundary is matched, we may want to update the height info to match
            // the previous one if have the same direction.
            if ( vec_major_coord.size() > 0 )
            {
               int num_thres = 10;

               printf("The boundary %d may match the following boundaries: ", i_outer);
               for (int i = 0; i < array_n(end_points); i ++ ) {
                  if ( st_match_rank[i] > num_thres ) {
                     printf("%d ", i);
                  }
               }
               printf("\n");

               // update the value of height based on dir
               int max_idx = -1, max_num = -1;
               for ( int i = 0; i < vec_major_coord.size(); i +=2 ) {
                  printf("COORD: %f, NUM: %d\n", vec_major_coord[i], (int)vec_major_coord[i+1]);
                  if ( vec_major_coord[i+1] > max_num ) {
                     max_num = (int) vec_major_coord[i+1];
                     max_idx = i;
                  }
               }

               vector<float> *vec_info = array_fetch(vector<float> *, z_arr, i_outer);
               int updated_dir_group = (int) vec_info->at(4);
               float base_new = vec_major_coord[max_idx];
               float base_old = vec_info->at(2);
               printf("Update the height info: [#%d] base from [%f --> %f]\n", i_outer, base_old, base_new);
               vec_info->at(2) = base_new;
               vec_info->at(3) = base_new;

               // update the all the boundary with the same base in the same direction group.
               vector<float> *vec_dir;
               int dir_indx;
               arrayForEachItem(vector<float> *, z_arr, dir_indx, vec_dir) {
                  if ( updated_dir_group == (int) vec_dir->at(4) ) {
                     vec_dir->at(2) = vec_dir->at(2) == base_old ? base_new : vec_dir->at(2);
                     vec_dir->at(3) = vec_dir->at(3) == base_old ? base_new : vec_dir->at(3);
                  }
               }

            }
         } // end of for all end_points
         
      }
      
      // second round - zipper face in different faces;
      // 2.2 - final matched the points in different faces and directions.
      // Steps:
      // 1. group matched points in the prevous step, e.g., two segments/groups should be identifed.
      // 2. compute the end points for each group or segment based on slope. (warning if index wrapping, i.e, from size() -> 0)
      // 3. match for each point inside of a segment
      // 4. transform unmatched points inside of a segment, so that the linear structure is kept.
      // Option step: update template points if needed. (for those points falling ouside the plane of new boundary).
      if ( 1 )
      {
         // free old end_points

         int set_num =  array_n(arr_matched_bnds);
         printf("\n=================== 2.5 round for different direcionts with %d sets. =============\n\n", set_num);

         vector<int> *vec_match_bnd;

         // order the segments in the same boundary to make sure later index go first
         printf("Reordering the segments in the same boundary.. \n");
         array_t *arr_matched_bnds_ordered = array_alloc(vector<int> *, 0);
         for ( int i = set_num - 1 ; i >= 0; i -- ) {
            vec_match_bnd =  array_fetch(vector<int> *, arr_matched_bnds, i);
            array_insert_last(vector<int> *, arr_matched_bnds_ordered, vec_match_bnd);
            int start_pt_idx  = vec_match_bnd->at(0);
            int end_pt_idx    = vec_match_bnd->at(1);
            int bnd_idx       = vec_match_bnd->at(2);
            printf(" SET %d: boundary: %d, start_pt: %d, end_pt: %d\n", set_num - i, bnd_idx, start_pt_idx, end_pt_idx);
         }

         arr_matched_bnds = arr_matched_bnds_ordered;
         arrayForEachItem(vector<int> *, arr_matched_bnds, i_outer, vec_match_bnd) {
            int start_pt_idx  = vec_match_bnd->at(0);
            int end_pt_idx    = vec_match_bnd->at(1);
            int bnd_idx       = vec_match_bnd->at(2);
            int match_bnd_idx = vec_match_bnd->at(3);
            int match_idx_dir = vec_match_bnd->at(4);
            array_t *arr_replacing_bnd = array_alloc(vector<float> *, 0);

            printf(" The SET %d: start: %d, end: %d, bnd_idx: %d, match_idx: %d, matched_dir: %d\n",
                   i_outer, start_pt_idx, end_pt_idx, bnd_idx, match_bnd_idx, match_idx_dir);

            vector<int> vec_mark( end_pt_idx - start_pt_idx + 1, 0 );
            {
               array_t *bnd_pts_new     = array_fetch(array_t *, end_points_new, bnd_idx);
               array_t *bnd_pts_matched = array_fetch(array_t *, end_points_new, match_bnd_idx);

               // compute matched points
               int pre_matched_pts_idx = -1;

               int dir = (int) array_fetch(vector<float> *, z_arr, i_outer)->at(0);
               int slope_coord_a = -1;
               int slope_coord_b = -1;
               int slope_coord_c = -1;
               if ( dir == 0 ) {
                  slope_coord_a = 0; // x
                  slope_coord_b = 2; // z
                  slope_coord_c = 1; // y
               }
               else if ( dir == 1 ) {
                  slope_coord_a = 0; // x
                  slope_coord_b = 1; // y
                  slope_coord_c = 2; // z
               }
               else if ( dir == 2 ) {
                  slope_coord_a = 2; // z
                  slope_coord_b = 1; // y
                  slope_coord_c = 0; // x
               }
               assert( slope_coord_a > -1 && slope_coord_b > -1 );
               
               int matched_pts_num   = 0;
               float matched_total_a = 0.0;
               float matched_total_b = 0.0;
               float matched_c_the_same = -101010.0;

               // add previous points
               for ( int i = 0; i < start_pt_idx; i ++ )
               {
                  vector<float> *pt = array_fetch(vector<float> *, bnd_pts_new, i);
                  vector<float> *cpy_pt = new vector<float> (*pt);
                  array_insert_last(vector<float> *, arr_replacing_bnd, cpy_pt);
               }

               // update matched points
               for ( int i = start_pt_idx; i <= end_pt_idx; i ++ )
               {
                  vector<float> *cur_pt = array_fetch(vector<float> *, bnd_pts_new, i);
                  float x = cur_pt->at(0);
                  float y = cur_pt->at(1);
                  float z = cur_pt->at(2);
                  
                  int i_inner;
                  vector<float> *points;
                  int matched_pts_idx = -1;
                  float min_dist = 10000.0;
                  const float dist_thres = 2.0;  // This is a hard problem
                  arrayForEachItem(vector<float> *, bnd_pts_matched, i_inner, points) {
                     float dist = sqrt(abs(x - (*points)[0])*abs(x - (*points)[0]) +
                                       abs(y - (*points)[1])*abs(y - (*points)[1]) +
                                       abs(z - (*points)[2])*abs(z - (*points)[2]));
                     if ( dist > dist_thres )
                        continue;
                     else {
                        if ( dist < min_dist ) {
                           min_dist = dist;
                           matched_pts_idx = i_inner;
                        }
                     }
                  }

                  if ( matched_pts_idx != -1 ) {

                     //printf("== Found match for INDEX %d, with %d, ==\n", i, matched_pts_idx);
                     int offset = matched_pts_idx - pre_matched_pts_idx;
                     int sign   = offset > 0 ? 1 : offset < 0 ? -1 : 0;
                     if ( pre_matched_pts_idx == -1 ) {
                        offset = match_idx_dir;
                     }

                     vec_mark[ i - start_pt_idx ] = 2;
                     if ( match_idx_dir == sign ) {

                        vector<float> *pt_matched = array_fetch(vector<float> *, bnd_pts_matched, matched_pts_idx);
                        vector<float> *pt_new = cur_pt;
                        matched_pts_num ++;
                        matched_total_a += pt_new->at(slope_coord_a) - pt_matched->at(slope_coord_a);
                        matched_total_b += pt_new->at(slope_coord_b) - pt_matched->at(slope_coord_b);

                        if ( matched_c_the_same == -101010.0 )
                           matched_c_the_same = pt_matched->at(slope_coord_c);
                        
                        if ( abs(offset) < 5 ) {

                           // here, let's empty the vector first.
                           pt_new->clear();

                           // if the first point, just replace it
                           
                           // ISSUE: the point matched may not be in the same plane. ......
                           
                           if ( pre_matched_pts_idx == -1 ) {
                              pt_new->push_back(pt_matched->at(0));
                              pt_new->push_back(pt_matched->at(1));
                              pt_new->push_back(pt_matched->at(2));
                              
                           } else {

                              for ( int i = pre_matched_pts_idx + sign; i != matched_pts_idx + sign; i += sign ) {
                                 vector<float> *pts_matched = array_fetch(vector<float> *, bnd_pts_matched, i);
                                 if ( pt_matched->at(slope_coord_c) != matched_c_the_same ) {
                                    printf("WWWWWARNIG::: the 3rd corrd is not the same [%f v.s. %f], speical handle is needed, which is not being implemented yet.\n", matched_c_the_same, pt_matched->at(slope_coord_c));
                                 }
                                 pt_new->push_back( pts_matched->at(0) );
                                 pt_new->push_back( pts_matched->at(1) );
                                 pt_new->push_back( pts_matched->at(2) );
                              }
                           }

                           // only add this point here.
                           vec_mark[ i - start_pt_idx ] = 1;
                           
                        } else {
                           printf("WARNING: matched pt is far away from previous matched points!!!! - [offset - %d]\n", abs(offset));
                        }

                     }
                     
                     pre_matched_pts_idx = matched_pts_idx;
                     
                  } else {
                     pre_matched_pts_idx = -1;
                  }
                    
               }

               // update extra points
               float ave_offset_a = matched_total_a/matched_pts_num;
               float ave_offset_b = matched_total_b/matched_pts_num;

               printf ("Total matched points: %d, ave_a: %f, ave_b: %f\n", matched_pts_num, matched_total_a, matched_total_b);
               for ( int i = start_pt_idx; (i_outer != 3) && i <= end_pt_idx; i ++ ) {
                  if ( vec_mark[i - start_pt_idx] == 0 ) {
                     printf("updated 0 VEC points at %d.\n", i - start_pt_idx);
                     vector<float> *cur_pt = array_fetch(vector<float> *, bnd_pts_new, i);
                     cur_pt->at(slope_coord_a) -= ave_offset_a;
                     cur_pt->at(slope_coord_b) -= ave_offset_b;
                     vector<float> *cpy_pt = new vector<float> (*cur_pt);
                     array_insert_last(vector<float> *, arr_replacing_bnd, cpy_pt);
                  }
                  else if ( vec_mark[i - start_pt_idx] == 1 ) {
                     vector<float> *cur_pt = array_fetch(vector<float> *, bnd_pts_new, i);
                     for (int pt_idx = 0; pt_idx < cur_pt->size(); pt_idx += 3)
                     {
                        vector<float> *cpy_pt = new vector<float>;
                        cpy_pt->push_back( (*cur_pt)[pt_idx  ] );
                        cpy_pt->push_back( (*cur_pt)[pt_idx+1] );
                        cpy_pt->push_back( (*cur_pt)[pt_idx+2] );
                        array_insert_last(vector<float> *, arr_replacing_bnd, cpy_pt);
                     }
                  }
               }

               // add the remaining points
               for ( int i = end_pt_idx + 1; i < array_n(bnd_pts_new); i ++ )
               {
                  vector<float> *pt = array_fetch(vector<float> *, bnd_pts_new, i);
                  vector<float> *cpy_pt = new vector<float> (*pt);
                  array_insert_last(vector<float> *, arr_replacing_bnd, cpy_pt);
               }
               
               // let's replace this boundary with new one
               array_insert(array_t *, end_points_new, bnd_idx, arr_replacing_bnd);

            } // end of one boundary

         } // end of for all end_points
         
      } // if (1)


      // Third round, compress straight lines
      if ( 1 )
      {
         printf("\n=================== 3 round for compress straight lines. =============\n\n");

         int k1, k2;
         array_t *vec_arr;
         vector<float> *vec_pts;
         arrayForEachItem(array_t *, end_points_new, k1, vec_arr) {

            vector<float> *vec_info = array_fetch(vector<float> *, z_arr, k1);

            int dir = (int)(*vec_info)[0];
            int slope_coord_a = -1;
            int slope_coord_b = -1;
            int slope_coord_c = -1;
            if ( dir == 0 ) {
               slope_coord_a = 0; // x
               slope_coord_b = 2; // z
               slope_coord_c = 1; // y
            }
            else if ( dir == 1 ) {
               slope_coord_a = 0; // x
               slope_coord_b = 1; // y
               slope_coord_c = 2; // z
            }
            else if ( dir == 2 ) {
               slope_coord_a = 2; // z
               slope_coord_b = 1; // y
               slope_coord_c = 0; // x
            }
            assert( slope_coord_a > -1 && slope_coord_b > -1 && slope_coord_c > -1 );

            array_t *vec_arr_new = array_alloc(vector<float> *, 0);
            int data_len = array_n(vec_arr);
            vector<float> *pre_pt, *next_pt, *cur_pt;
            arrayForEachItem(vector<float> *, vec_arr, k2, vec_pts) {
               if ( k2 == 0 || k2 == data_len - 1 ) {
                  pre_pt = vec_pts;
                  array_insert_last(vector<float>*, vec_arr_new, new vector<float> (*vec_pts));
                  continue;
               }

               bool canIgnored = false;
               next_pt = array_fetch(vector<float> *, vec_arr, k2+1 );
               if ( ( vec_pts->at(slope_coord_a) == pre_pt->at(slope_coord_a)
                      && pre_pt->at(slope_coord_a)  == next_pt->at(slope_coord_a) ) ||
                    ( vec_pts->at(slope_coord_b) == pre_pt->at(slope_coord_b)
                      && pre_pt->at(slope_coord_b)  == next_pt->at(slope_coord_b) ) )
               {
                  canIgnored = true;
               }

               if ( !canIgnored ) {
                  array_insert_last(vector<float>*, vec_arr_new, new vector<float> (*vec_pts));
                  pre_pt = vec_pts;
               }
            }

            // insert the substitute one
            array_insert(array_t *, end_points_new, k1, vec_arr_new);
         }

      } // if (1)
      
   } // if (1)

   // print out the new zipper points
   // ksd_print_out_pts_in_IR(end_points, z_arr);
   {
      FILE *fd = fopen(output_path, "w");
      assert(fd);
      
      int k1, k2;
      array_t *vec_arr;
      vector<float> *vec_pts;
      arrayForEachItem(array_t *, end_points_new, k1, vec_arr) {

         vector<float> *vec_info = array_fetch(vector<float> *, z_arr, k1);
         int dir = (int)(*vec_info)[0];
         int unit_type = (int) (*vec_info)[1];
         float base = (*vec_info)[2];
         float top  = (*vec_info)[3];
         int dir_group = (int)(*vec_info)[4];

         fprintf(fd, "BEGIN POLYGON\nDIRECTION\n%d %d\nHEIGHT\n", dir, dir_group);
         fprintf(fd, "%f %f\nPOINTS\n", base, top );
         float pre_f1, pre_f2;
         float f1, f2;
         arrayForEachItem(vector<float> *, vec_arr, k2, vec_pts) {
            if ( dir == 0 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[2];
            } else if ( dir == 1 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[1];
            } else {
               f1 = (*vec_pts)[2];
               f2 = (*vec_pts)[1];
            }

            // avoid duplicated point adjacent.
            if ( k2 != 0  && pre_f1 == f1 && pre_f2 == f2) {
               continue;
            }
            
            fprintf(fd, "%f %f\n", f1, f2);
            pre_f1 = f1;
            pre_f2 = f2;
         }

         if ( unit_type != 0 ){
            printf("NOT EXTRUSION? not handled yet!\n");
            assert(0);
         }
         
         fprintf(fd, "EXTRUSION\nEND POLYGON\n");
      }

      fclose(fd);
   }
   
}


void compute_projected_point_on_line(LINE_3D *line, POINT_3D &P, POINT_3D &Pb)
{
   POINT_3D v, w;
   POINT_3D L_P0 = line->P0;
   POINT_3D L_P1 = line->P1;
   v.x = L_P1.x - L_P0.x;
   v.y = L_P1.y - L_P0.y;
   v.z = L_P1.z - L_P0.z;
   w.x = P.x - L_P0.x;
   w.y = P.y - L_P0.y;
   w.z = P.z - L_P0.z;
   
   float c1 = dot_product(w,v);
   float c2 = dot_product(v,v);
   float b = c1 / c2;
      
   Pb.x = L_P0.x + b * v.x;
   Pb.y = L_P0.y + b * v.y;
   Pb.z = L_P0.z + b * v.z;
}

// Function: ksd_do_zippering_for_each_facades
//
// Assumption: The point far away from the line is not part of the zippering
//             This requires the threshold DIST is relative big
//
void ksd_do_zippering_for_each_facades(LINE_3D *line,
                                       array_t *end_points,
                                       int idx_face,
                                       float thres = 0.5)
{
   const float DIST = thres == 0.5 ? 2.0 : thres; // how to know this? THRESHOLD: 
   const int num_pts_slope = 5;  // the average slope based on this many observation.
   
   // here, we will update end_points's boundary of indx_face
   assert(idx_face < array_n(end_points) );
   array_t *bnd_pts = array_fetch(array_t *, end_points, idx_face);
   array_t *bnd_pts_new = array_alloc(vector<float> *, 0);

   int k;
   vector<float> *pt;
   int total_num = 0;
   POINT_3D p;
   int increase_index = -1;
   int decrease_index = -1;
   bool first_point_included = false;
   bool first_change_happened = false;
   int stable_num = 15;
   arrayForEachItem(vector<float> *, bnd_pts, k, pt) {
      p.x = pt->at(0);
      p.y = pt->at(1);
      p.z = pt->at(2);

      if ( compute_dist_point_to_line(p, line->P0, line->P1) < DIST ) {
         if ( k == 0 )
            first_point_included = true;

         if ( first_point_included && first_change_happened && k - increase_index > stable_num ) {
            decrease_index = k;
            break;
         }

         if ( !first_point_included && !first_change_happened ) {
            first_change_happened = true;
            decrease_index = k;
         }
         
      } else {
         if ( first_point_included && !first_change_happened ) {
            first_change_happened = true;
            increase_index = k;
         }

         if ( !first_point_included && first_change_happened && k - decrease_index > stable_num ) {
            increase_index = k;
            break;
         }
      }
      
   }


   printf("Total %d points from FACE #%d, entering(decrease_index): %d,exiting(increase_index): %d, first is in? %s\n",
          array_n(bnd_pts),
          idx_face,
          decrease_index,
          increase_index,
          first_point_included ? "TRUE" : "FALSE");

   // for increase_index, compute the average slope and intersection point
   /*
   float slope1 = ksd_average_slope(increase_index, bnd_pts, num_pts_slope, 1);
   POINT_3D *int_pt1 = ksd_compute_intersection_pt_from_slope(line, pt, slope);
   */

   POINT_3D project_pt_inc, p1;
   pt = array_fetch(vector<float> *, bnd_pts, increase_index);
   p1.x = pt->at(0);
   p1.y = pt->at(1);
   p1.z = pt->at(2);
   compute_projected_point_on_line(line, p1, project_pt_inc);

   POINT_3D project_pt_dec;
   pt = array_fetch(vector<float> *, bnd_pts, decrease_index);
   p1.x = pt->at(0);
   p1.y = pt->at(1);
   p1.z = pt->at(2);
   compute_projected_point_on_line(line, p1, project_pt_dec);

   
   {
      pt = new vector<float> ();
      pt->push_back(project_pt_inc.x);
      pt->push_back(project_pt_inc.y);
      pt->push_back(project_pt_inc.z);
      array_insert_last(vector<float> *, bnd_pts_new, pt);

      int pt_i = increase_index;
      while ( true ) {
         pt = array_fetch(vector<float> *, bnd_pts, pt_i);
         array_insert_last(vector<float> *, bnd_pts_new, pt );

         pt_i = (pt_i + 1) % array_n(bnd_pts);
         
         if ( pt_i == (decrease_index + 1) ) {
            break;
         }
      }

      pt = new vector<float> ();
      pt->push_back(project_pt_dec.x);
      pt->push_back(project_pt_dec.y);
      pt->push_back(project_pt_dec.z);
      array_insert_last(vector<float> *, bnd_pts_new, pt);
      
   }

   array_insert(array_t *, end_points, idx_face, bnd_pts_new);
   printf("FACE #%d has total points %d -> %d\n", idx_face, array_n(bnd_pts), array_n(bnd_pts_new));
      
}


// Command:
//     Project.exe -K -O "-A 6 -P IR_all.txt -O ./IR_update_all.txt -B <cut of distance>"
//
void ksd_zipper_boundarya_app_intersection_line(void **params)
{
   // decode the parameters
   bool db = false; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_path  = (char *) params[3];
   int thres_plane_region = (int) params[4];
   char *prefix_dir = (char *) params[5];
   float thres_region = *((float *) params[6]);

   if ( !prefix_dir ) {
      prefix_dir = "NO_DIR";
   }

   // load the IR file
   array_t *z_arr = array_alloc(vector<float> *, 0);
   array_t *end_points = load_end_points_from_IR_with_dirs(input_prefix, z_arr);
   array_t *end_points_new = array_alloc(array_t *, 0);
   array_t *arr_matched_bnds = array_alloc(array_t *, 0);

   vector<LINE_3D *> vec_lines;
   map<LINE_3D *, vector<int> *> map_line2Face;
   
   // compute the intersection line
   {
      // get the 3 points from the list of the boundary
      // the first point is the 1st point,
      // the second point is the point farest to the 1st point.
      // the third point is the middle point of first and second point
      int k;
      array_t *arr_pts;
      array_t *arr_key_pts = array_alloc(vector<POINT_3D> *, 0);
      arrayForEachItem(array_t *, end_points, k, arr_pts) {

         POINT_3D p1, p2, p3;
         int k1;
         vector<float> *pt;
         float max_dist = 0.0;
         int max_index = -1;
         arrayForEachItem(vector<float> *, arr_pts, k1, pt) {
            if ( k1 == 0 ) {
               p1.x = (*pt)[0];
               p1.y = (*pt)[1];
               p1.z = (*pt)[2];
               continue;
            }

            float dist = sqrt( (p1.x - (*pt)[0]) * (p1.x - (*pt)[0]) +
                               (p1.y - (*pt)[1]) * (p1.y - (*pt)[1]) +
                               (p1.z - (*pt)[2]) * (p1.z - (*pt)[2]) );

            if ( dist > max_dist ) {
               max_dist = dist;
               p3.x = (*pt)[0];
               p3.y = (*pt)[1];
               p3.z = (*pt)[2];
               max_index = k1;
            }
         }

         int mid_i = max_index / 2;
         assert(mid_i > 0 && mid_i < max_index);
         vector<float> *mid_pt = array_fetch(vector<float> *, arr_pts, mid_i);
         p2.x = (*mid_pt)[0];
         p2.y = (*mid_pt)[1];
         p2.z = (*mid_pt)[2];

         printf("Boundary #%d Key points: P0[%.2f, %.2f, %.2f], P1[%.2f, %.2f, %.2f], P2[%.2f, %.2f, %.2f]\n",
                k, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z);
         vector<POINT_3D> *kpt = new vector<POINT_3D>;
         kpt->push_back(p1);
         kpt->push_back(p2);
         kpt->push_back(p3);
         array_insert_last(vector<POINT_3D> *,arr_key_pts, kpt);
      }

      vector<POINT_3D> *kpt;
      arrayForEachItem(vector<POINT_3D> *, arr_key_pts, k, kpt) {
         vector<POINT_3D> *pl2 = array_fetch(vector<POINT_3D> *, arr_key_pts, (k+1)%array_n(arr_key_pts));

         POINT_3D p1 = (*kpt)[0];
         POINT_3D p2 = (*kpt)[1];
         POINT_3D p3 = (*kpt)[2];
         POINT_3D p4 = (*pl2)[0];
         POINT_3D p5 = (*pl2)[1];
         POINT_3D p6 = (*pl2)[2];
         printf("PLANE1: P0[%.2f, %.2f, %.2f], P1[%.2f, %.2f, %.2f], P2[%.2f, %.2f, %.2f]\n", p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z);
         printf("PLANE2: P0[%.2f, %.2f, %.2f], P1[%.2f, %.2f, %.2f], P2[%.2f, %.2f, %.2f]\n", p4.x, p4.y, p4.z, p5.x, p5.y, p5.z, p6.x, p6.y, p6.z);

         PLANE_3D *p3d1 = compute_plane_from_3D_pts(p1, p2, p3);
         PLANE_3D *p3d2 = compute_plane_from_3D_pts(p4, p5, p6);

         POINT_3D P0, P1;
         POINT_3D *vec = compute_intersection_line_from_2_planes(*p3d1, *p3d2, P0, P1);
         LINE_3D *line = (LINE_3D *) malloc(sizeof(LINE_3D));
         line->P0 = P0;
         line->P1 = P1;
         printf("INTERSECTION LINE IS P0: [%f, %f, %f]\n",P0.x, P0.y, P0.z);
         printf("INTERSECTION LINE IS P1: [%f, %f, %f]\n",P1.x, P1.y, P1.z);

         vector<int> *face_index = new vector<int> ();
         face_index->push_back(k);
         face_index->push_back((k+1)%array_n(arr_key_pts));
         vec_lines.push_back(line);
         map_line2Face[line] = face_index;
      }
   }

   // now zipper the boundary points along the intersection lines
   {
      // for each shared line, get the facades that close to it. and do the zippering one by one
      for (int i = 0; i < vec_lines.size(); i ++ ) {
         LINE_3D *line = vec_lines[i];
         vector<int> *face_index = map_line2Face[line];
         for (int j = 0; j < face_index->size(); j ++ ) {
            int idx_face = face_index->at(j);
            printf("Processing LINE[%.2f, %.2f, %.2f -> [%.2f, %.2f, %.2f ] with FACE #%d.\n",
                   line->P0.x, line->P0.y, line->P0.z,
                   line->P1.x, line->P1.y, line->P1.z, idx_face);
            ksd_do_zippering_for_each_facades(line, end_points, idx_face, thres_region);
         }
      }
   }
   
   // print out the new zipper points
   // ksd_print_out_pts_in_IR(end_points, z_arr);
   if (1)
   {
      FILE *fd = fopen(output_path, "w");
      assert(fd);
      
      int k1, k2;
      array_t *vec_arr;
      vector<float> *vec_pts;
      arrayForEachItem(array_t *, end_points, k1, vec_arr) {

         vector<float> *vec_info = array_fetch(vector<float> *, z_arr, k1);
         int dir = (int)(*vec_info)[0];
         int unit_type = (int) (*vec_info)[1];
         float base = (*vec_info)[2];
         float top  = (*vec_info)[3];
         int dir_group = (int)(*vec_info)[4];
         int type = (int)(*vec_info)[5];

         fprintf(fd, "BEGIN POLYGON\nDIRECTION\n%d %d\nHEIGHT\n", dir, dir_group);
         fprintf(fd, "%f %f\nTYPE\n%d\nPOINTS\n", base, top, type );
         float pre_f1, pre_f2;
         float f1, f2;
         arrayForEachItem(vector<float> *, vec_arr, k2, vec_pts) {
            if ( dir == 0 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[2];
            } else if ( dir == 1 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[1];
            } else {
               f1 = (*vec_pts)[2];
               f2 = (*vec_pts)[1];
            }

            // avoid duplicated point adjacent.
            if ( k2 != 0  && pre_f1 == f1 && pre_f2 == f2) {
               continue;
            }
            
            if ( type == 2 ) {
               fprintf(fd, "%f %f %f\n", (*vec_pts)[0], (*vec_pts)[1], (*vec_pts)[2]);
            } else {
               fprintf(fd, "%f %f\n", f1, f2);
            }
            
            pre_f1 = f1;
            pre_f2 = f2;
         }

         if ( unit_type != 0 ){
            printf("NOT EXTRUSION? not handled yet!\n");
            assert(0);
         }
         
         fprintf(fd, "EXTRUSION\nEND POLYGON\n");
      }

      fclose(fd);
   }
   
}


// Command:
//     Project.exe -K -O "-A 6 -P IR_all.txt -O ./IR_update_all.txt"
//
void ksd_zipper_boundary(void **params)
{
   int approach = 0;

   if ( approach == 0 ) {
      ksd_zipper_boundarya_app_intersection_line(params);
   } else {
      ksd_zipper_boundarya_app_match(params);
   }
}


// Function: ksd_extrusion_intersection
//
// Description: Compute the intersection plane of two extruded models
//
// Command:
//     Project.exe -K -O "-A 7 -P IR_all_roof.txt -O ./IR_part_roof_only.txt"
//
// Algorithm:
//
//
void ksd_extrusion_intersection(void **params)
{
   // decode the parameters
   bool db = false; // debug?
   char *input_prefix  = (char *) params[0];
   int left_num  = (int ) params[1];
   int right_num = (int ) params[2];
   char *output_path  = (char *) params[3];
   int thres_plane_region = (int) params[4];
   char *prefix_dir = (char *) params[5];

   if ( !prefix_dir ) {
      prefix_dir = "NO_DIR";
   }

   // load the IR file
   array_t *z_arr = array_alloc(vector<float> *, 0);
   array_t *end_points = load_end_points_from_IR_with_dirs(input_prefix, z_arr);

   // some information about the model
   // map contains the intersection models
   map<int, int> map_intersect;
   map_intersect[32+1] = 8;  // assume this is known.
   map_intersect[32+2] = 8;
   map_intersect[32+3] = 8;
   map_intersect[32+4] = 8;
   map_intersect[32+5] = 8;
   map_intersect[32+6] = 8;
   map_intersect[32+7] = 8;
   map_intersect[32+8] = 8;
   map_intersect[32+9] = 8;
   map_intersect[32+10] = 8;
   int shared_axis = 2;   // z axis is shared by ir.first and ir.second
   int toComp_axis = 0;   // x axis is to be computed
   int toCopy_axis = 1;   // y axis is to be copied since it extrudes along X axis.
   vector<int> push_order;
   push_order.push_back(toComp_axis);
   push_order.push_back(toCopy_axis);
   push_order.push_back(shared_axis);
   
   map<int, int>::iterator ir;
   map<array_t *, array_t *> map_taper_bnds;
   for (ir =  map_intersect.begin(); ir != map_intersect.end(); ir ++ ) {
      printf("Region %d v.s. Region %d\n", (*ir).first, (*ir).second);
      int new_bnd_idx = (*ir).first;
      int old_bnd_idx = (*ir).second;

      array_t *new_bnd = array_fetch(array_t *, end_points, new_bnd_idx);
      array_t *old_bnd = array_fetch(array_t *, end_points, old_bnd_idx);
      array_t *new_bnd_taper = array_alloc(vector<float> *, 0);
      map_taper_bnds[new_bnd] = new_bnd_taper;

      vector<float> *vec_info = array_fetch(vector<float> *, z_arr, new_bnd_idx);
      int dir = (int)(*vec_info)[0];
      int unit_type = (int) (*vec_info)[1];
      float base = (*vec_info)[2];
      float top  = (*vec_info)[3];

      int k1, k2;
      vector<float> *pt_new, *pt_old;
      arrayForEachItem(vector<float> *, new_bnd, k1, pt_new) {

         vector<float> *pt_pre, *pt_cur, *pt_tapered;
         float match_value = pt_new->at(shared_axis);

         vector<vector<float> *> vec_candidate;
         arrayForEachItem(vector<float> *, old_bnd, k2, pt_old) {
            if ( k2 == 0 ) {
               pt_pre = pt_old;
               continue;
            }

            float v_min = min(pt_old->at(shared_axis), pt_pre->at(shared_axis));
            float v_max = max(pt_old->at(shared_axis), pt_pre->at(shared_axis));
            
            if ( match_value >= v_min && match_value < v_max ) {
               vec_candidate.push_back(pt_pre);
               vec_candidate.push_back(pt_old);
            }

            pt_pre = pt_old;
         }

         // find the right one among candidates
         bool found = false;
         pt_tapered = new vector<float> ();
         if ( vec_candidate.size() >= 2 ) {
            found = true;

            if ( vec_candidate.size() == 2 )
            {
               pt_pre = vec_candidate[0];
               pt_cur = vec_candidate[1];
            } else {
               int right_index = -1;
               float shortest_dist = 1000000.0;

               // compute the right candidate.
               for (int i = 0; i < vec_candidate.size(); i += 2) {
                  float tmp_dist = abs((*pt_new)[0] - (*vec_candidate[i])[0]) +
                                   abs((*pt_new)[1] - (*vec_candidate[i])[1]) +
                                   abs((*pt_new)[2] - (*vec_candidate[i])[2]);
                       tmp_dist += abs((*pt_new)[0] - (*vec_candidate[i+1])[0]) +
                                   abs((*pt_new)[1] - (*vec_candidate[i+1])[1]) +
                                   abs((*pt_new)[2] - (*vec_candidate[i+1])[2]);

                       if ( tmp_dist < shortest_dist ) {
                          right_index = i;
                          shortest_dist = tmp_dist; 
                       }
               }

               assert( right_index > -1 );
               pt_pre = vec_candidate[right_index];
               pt_cur = vec_candidate[right_index+1];
                     
            }
            
            // compute the coordinated for tapered point
            float ratio = 0.0, diff_pt = 0.0;
            if ( match_value - pt_pre->at(shared_axis) != 0 ) {
               ratio = (match_value - pt_pre->at(shared_axis)) / (pt_cur->at(shared_axis) - pt_pre->at(shared_axis));
               diff_pt = (pt_cur->at(toComp_axis) - pt_pre->at(toComp_axis)) * ratio;

               if ( abs(ratio) > 1 ) {
                  printf("????: Ratio %f should be less or equal to 1\n ???", ratio);
               }
            }
            
            float val_vec[3];
            val_vec[toComp_axis] = pt_pre->at(toComp_axis) + diff_pt;
            val_vec[toCopy_axis] = pt_new->at(toCopy_axis);
            val_vec[shared_axis] = pt_new->at(shared_axis);

            pt_tapered->push_back( val_vec[push_order[0]] );
            pt_tapered->push_back( val_vec[push_order[1]] );
            pt_tapered->push_back( val_vec[push_order[2]] );
            
         }

         if ( !found ) {
            
            pt_tapered->push_back( (*pt_new)[3] );
            pt_tapered->push_back( (*pt_new)[4] );
            pt_tapered->push_back( (*pt_new)[5] );
         }

         assert( pt_tapered->size() == 3);
         array_insert_last(vector<float> *, new_bnd_taper, pt_tapered);
      }

   }
   printf("Total %d Mappings\n", map_intersect.size());

   // dump the result of the computation
   {
      
      FILE *fd = fopen(output_path, "w");
      assert(fd);
      
      int k1, k2;
      array_t *vec_arr;
      vector<float> *vec_pts;
      arrayForEachItem(array_t *, end_points, k1, vec_arr) {

         vector<float> *vec_info = array_fetch(vector<float> *, z_arr, k1);
         int dir = (int)(*vec_info)[0];
         int unit_type = (int) (*vec_info)[1];
         float base = (*vec_info)[2];
         float top  = (*vec_info)[3];
         int dir_group = (int)(*vec_info)[4];

         array_t *arr_tapered;
         if ( map_taper_bnds.find(vec_arr) != map_taper_bnds.end() ) {
            unit_type = 1;
            arr_tapered = map_taper_bnds[vec_arr];
            assert( array_n(arr_tapered) == array_n(vec_arr) );
         }

         fprintf(fd, "BEGIN POLYGON\nDIRECTION\n%d %d\nHEIGHT\n", dir, dir_group);
         fprintf(fd, "%f %f\nTYPE\n%d\nPOINTS\n", base, top, unit_type );
         float pre_f1, pre_f2;
         float f1, f2;
         arrayForEachItem(vector<float> *, vec_arr, k2, vec_pts) {
            if ( dir == 0 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[2];
            } else if ( dir == 1 ) {
               f1 = (*vec_pts)[0];
               f2 = (*vec_pts)[1];
            } else {
               f1 = (*vec_pts)[2];
               f2 = (*vec_pts)[1];
            }

            // avoid duplicated point adjacent.
            if ( k2 != 0  && pre_f1 == f1 && pre_f2 == f2) {
               printf("ERROR: there is duplicated points at %d, [%f, %f]\n", k2, f1, f2);
               exit(0);
            }

            if ( unit_type == 0 ) {
               fprintf(fd, "%f %f\n", f1, f2);
            } else {
               fprintf(fd, "%f %f %f\n", (*vec_pts)[0], (*vec_pts)[1], (*vec_pts)[2]);
            }
            
            pre_f1 = f1;
            pre_f2 = f2;
         }

         if ( unit_type != 0 ){
            printf("This is a tapered model, NO. %d\n", k1);
            fprintf(fd, "TAPERED\n");
            arrayForEachItem(vector<float> *, arr_tapered, k2, vec_pts) {
               fprintf(fd, "%f %f %f\n", (*vec_pts)[0], (*vec_pts)[1], (*vec_pts)[2]);
            }
            
         } else {
            fprintf(fd, "EXTRUSION\n");
         }
         fprintf(fd, "END POLYGON\n");
      }

      fclose(fd);
   }

}



/*
  The overview of the reconstruction of one face:
  input: the slices
  output: the model shown in SketchUp

  1. find the right number of integration, e.g., face_inside: 81 - 89
  Project.exe -J -O "-s 81 -e 90 -r result/point_cloud_process_1000/slices_1000_1000_2048_face_inside/image_slice -w result/point_cloud_process_1000/slices_0000_ksd_fuse_all/test/image_slice"

  2. crop the image obtained in step 1 by removing all unrelated part - only keep the walls and windows inside these walls.

  This is hard to detect and model windows or doors generically. ie. for a lot of case, it can not be extended to any cases.
  On the other hand, we can try to detect the change of CC along the vertical directions. Let's try this out on roof of GCT

  manually work involved to get the best result (think of the data as simulated good data):
  *. for each facade, manually remove noise/fill holes to get the best WALL.
  *. indicate some similarity among adjacent windows/doors/extrusions

  *** It seems infer wall/windows(doors)/extrusion is very hard at this point, maybe another paper?

  *** How about combining keyslice detection with windows(doors) inference?
      *. detect the unchanged part using pixel based matching, like the above function.
      *. find the windows, ignore it during keyslice detection so that the whole roof can be viewed as two
         extruded parts: arc part + windows extrusion

  *** where to start, how to start and what's the goal?
      *. infer the wall, and then the windows/doors
         a. locate the related slices (reference, start and end index of the slice)
         b. noise removal - manually remove noise or fill holes for these slices.
         c. image what the model should be if without noise or holes!!!
      *. zipper the vertices or walls along each facades.
      *. finally, infer the extrusion part of the facade.
      *. show the whole model in sketchup


  *** the flow leading to the result
      *. automatically infer the wall use ksd_all_info_fusion() step 1. manually enhance it to get integrated image p1.
      *. manually enhance p1 by remove noise (cut with a window box) to get p2.
      *. use BPA BPA_boundary to get the boundary, b1, of the wall from p2, and compute all pixels inside this closed polygon.
      *. enhance p2 by superimpose the boundary b1 onto it and get the p3, this may close the boundary of windows/doors in p2.
      *. find all windows/doors inside the wall boundary b1, use similarity to enhance the windows with occlusion or missing data.
         to do above, integrate all w/d's, weight each pixel based on # of hits (one run). And then compute weights for each w/d
         for another run. The highest score is the best one (most matched), use this as the standard w/d for all similiar ones.
      *. get the blank areas after filling wall and windows.
         do this using the implemented method of watershed, obtain the bounding box for each region.
         that is, it is a range of height and intersection of face-inside or left-right.
         do ksd inside this small regions after inferring the direction of the regions, ie, horizontal or vertical regions.
      *. zipper the boundary of each fascade.
         to do this, we have to compute each 3D coordinates and confidence for each boundary. zipper low confidence
         face onto higher confidence face. Also, we may need to apply CEGAL to get the intersection of one structure blocked by another.

  *** the algorithm behind the feature
      *. how to zipper:
         compute the two planes, and intersection lines, and then compute points close to the line, merge it with the point perperticular to the line.
      *. how to compute the windows/doors depth info:
         use the similarity measurement.
      *. 

 */

