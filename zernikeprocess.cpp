#include "zernikeprocess.h"
#include <opencv/cv.h>
#include <cmath>
#include "mainwindow.h"
#include <QDebug>
#include "surfaceanalysistools.h"
std::vector<bool> zernEnables;
std::vector<double> zNulls;
double BestSC = -1.;
int Zw[] = {								/*  n    */
    1,									  //    0
    4,4,3,								  //	1
    6,6,8,8,5,  //8						  //	2
    8,8,10,10,12,12,7, // 15					3
    10,10,12,12,14,14,16,16,9,  //24			4
    12,12,14,14,16,16,18,18,20,20,11,  //35		5
    14,14,16, // 38
    // from here on we use only spherical terms so the number of the term is wrong
    13, 15,17  // for terms 39 40 41 but are really the next symetrical terms
    // the correct coefficient would be for the non symetrical terms ;
    //16,18,18,20,20,22,22,24,24,13
};

/*
Public Function ZernikeW(n As Integer) As Double
' N is Zernike number as given by James Wyant
' routine calculates the inverse of the weight in the variance computation
*/
int ZernikeW(int n)
{
    return(Zw[n]);
}

long fact( int n1, int n2)
{
    int f = n1;
    for (int i = n1-1; i > n2; --i)
    {
        f *= i;
    }
    return f;

}
void gauss_jordan(int n, double* Am, double* Bm)
{
    /*
Private Sub GJ(n As Integer)
Dim indxc(50) As Integer, indxr(50) As Integer, ipiv(50) As Integer
Dim i As Integer, icol As Integer, irow As Integer, j As Integer
Dim k As Integer, l As Integer, ll As Integer
Dim big As Double, dum As Double, pivinv As Double, temp As Double

'solves the equation b=ax
   'N is matrix order
   'on entry, Bm(1 to N) is the left-hand-side column vector
   'on exit, Bm(1 to N) is the solution vector, x
   'on entry, Am(1 to N, 1 to N) is the matrix coefficients
   'on exit, Am is the inverse matrix
 */
    int* ipiv = new int[n];
    int* indxr = new int[n];
    int* indxc = new int[n];
    double big;
    int irow;
    int icol;
    double pivinv;

    for(int  j = 0; j < n; ++j)
    {
        ipiv[j] = 0;
    }

    for(int i = 0; i < n; ++i)
    {
        big = 0.;
        for(int  j = 0; j < n; ++j)
        {
            if ((ipiv[j] != 1))
            {
                int row_ndx = j * n;
                for(int k = 0; k < n; ++k)
                {
                    if (ipiv[k] == 0)
                    {
                        int ndx = row_ndx + k;
                        if (fabs(Am[ndx]) >= big)
                        {
                            big = fabs(Am[ndx]);
                            irow = j;
                            icol = k;
                        }
                    }
                }

            }
        }
        ++ipiv[icol];
        if (irow != icol)
        {
            for(int l = 0; l < n; ++l)
            {
                int irow_ndx = irow * n + l;
                int icol_ndx = icol * n + l;
                double temp = Am[irow_ndx];
                Am[irow_ndx] = Am[icol_ndx];
                Am[icol_ndx] = temp;
            }
            double temp = Bm[irow];
            Bm[irow] = Bm[icol];
            Bm[icol] = temp;
        }
        indxr[i] = irow;
        indxc[i] = icol;
        pivinv = 1. / Am[icol * n + icol];
        Am[icol * n + icol] = 1.;
        for(int l = 0; l < n; ++l)
        {
            int ndx = icol * n + l;
            Am[ndx] *= pivinv;
        }
        Bm[icol] *= pivinv;
        for(int ll = 0; ll < n; ++ll)
        {
            if (ll != icol)
            {
                int ll_ndx = ll * n;
                double dum = Am[ll_ndx + icol];
                Am[ll_ndx + icol] = 0.;
                for( int l = 0; l < n; ++l)
                {
                    int ndx = ll * n + l;
                    Am[ndx] = Am[ndx] - Am[icol * n +  l] * dum;
                }
                Bm[ll] = Bm[ll] - Bm[icol] * dum;
            }
        }
    }
    for(int l = n-1; l>= 0; --l)
    {
        if (indxr[l] != indxc[l])
        {
            for(int k = 0; k < n; ++k)
            {
                double temp = Am[k * n + indxr[l]];
                Am[k * n + indxr[l]] = Am[k * n + indxc[l]];
                Am[k * n + indxc[l]] = temp;
            }
        }
    }
    delete[] ipiv;
    delete[] indxr;
    delete[] indxc;
}
/*
Public Function Zernike(n As Integer, X As Double, Y As Double) As Double
' N is Zernike number as given by James Wyant
*/

double Zernike(int n, double X, double Y)
{
    static double X2 = 0., X3 = 0., X4 = 0.;
    static double Y2 = 0., Y3 = 0., Y4 = 0.;
    static double R2 = 0.;
    static double last_x = 0.;
    static double last_y = 0.;

    if (last_x != X || last_y != Y)
    {
        X2 = X * X;
        X3 = X2 * X;
        X4 = X2 * X2;
        last_x = X;
        Y2 = Y * Y;
        Y3 = Y2 * Y;
        Y4 = Y2 * Y2;
        R2 = X2 +Y2;


        last_y =Y;
    }

    double d;
    switch(n)
    {
    case 0:
        //	return(.01 * sin(3 * M2PI * sqrt(R2)));
        return(1.);
        break;
    case 1:
        return(X);
        break;
    case 2:
        return(Y);
        break;
    case 3:
        return(-1. + 2. * R2);
        break;
    case 4:
        return(X2 - Y2);
        break;
    case 5:
        return(2. * X * Y);
        break;
    case 6:
        return(-2. * X + 3. * X * R2);
        break;
    case 7:
        return(-2. * Y + 3. * Y * R2);
        break;
    case 8:
        return(1.+ R2 * (-6. + 6. * R2));
        break;
    case 9:
        return(X3 - 3. * X * Y2);
        break;
    case 10:
        return(3. * X2 * Y - Y3);
        break;
    case 11:
        return(-3. * X2 + 4.* X4 + 3. * Y2- 4. * Y4);
        break;
    case 12:
        return 2. * X * Y*(-3. + 4. * R2);
        break;
    case 13:
        return X * (3. + R2 * (-12. + 10. * R2));
        break;
    case 14:
        return Y * (3. + R2 * (-12. + 10. * R2));
        break;
    case 15:		//2nd spherical
        return(-1. + R2 * (12. + R2* (-30. + 20. * R2)));
        break;
    case 16:
        return(X4 - 6. * X2 * Y2 + Y4);
        break;
    case 17:
        return 4. * X * Y * (X2- Y2);
        break;
    case 18:
        return X * (5. * X4 + 3. * Y2 *(4.-5.* Y2) - 2 * X2 * ( 2. + 5. * Y2));
        break;
    case 19:
        return Y * (15. * X4 + 4. * Y2  - 5. * Y4 + 2. * X2 * (-6. + 5. * Y2 ));
        break;
    case 20:
        return (X2 - Y2)*(6. + R2 * (-20. + 15.* R2));
        break;
    case 21:
        return       2. * X*Y*(6. + R2 * (-20.  + 15. * R2));
        break;
    case 22:
        return X * (-4. + R2 * (30. + R2 * (-60. + 35. * R2)));
        break;
    case 23:
        return Y * (-4. + R2 * (30. + R2 * (-60. + 35. * R2)));
        break;
    case 24:  //3Rd spherical
        return(1. + R2 * (-20. + R2 * (90. + R2 * (-140. + 70. * R2))));
        break;
    case 25:
        return X *( X4 - 10. * X2 * Y2 + 5. * Y4);
        break;
    case 26:
        d = Y*(5. * X4 - 10. * X2* Y2 + Y4);
        break;
    case 27:
        d = 6.* X4 * X2 - (30. * X2 * Y2) * (-1. + Y2) + Y4 * (-5. + 6. * Y2) - 5* X4 *(1.+6. * Y2);
        break;
    case 28:
        d = X*(-20. *  X2 * Y + 20. * Y3 + 24. * X2 * Y * R2 -	24 * Y3 * R2);
        break;
    case 29:
        d = X*( 10. * X2 - 30. * Y2 + R2 *(-30. * X2 + 90.* Y2 + R2 * (21.*X2 - 63.* Y2)));
        break;
    case 30:
        d = Y*(-10. * Y2 + 30. * X2 + R2 * (30. * Y2   - 90.* X2 + R2 * (-21.* Y2 + 63.* X2)));
        break;
    case 31:
        d = (-10. + R2 * (60. + R2* (-105. + 56. * R2)))* (X2 - Y2);
        break;
    case 32:
        d =  X*Y*(-20. + R2 * (120. + R2* (- 210. + 112.* R2)));
        break;
    case 33:
        d = X*(5. + R2 * (-60. + R2*(210. + R2 *(-280. + R2 * 126))));
        break;
    case 34:
        d = Y*(5. +R2 * (-60. + R2 * (210. +R2 * (-280. + 126.* R2))));
        break;
    case 35: //4 spherical
        d = -1. + R2 * (30. + R2 * (-210.  + R2 * (560. +R2 * (-630. + 252.* R2))));
        break;
    case 36:
        d = X4 * X2 - 15. * X4 * Y2 + 15. * X2 * Y4 - Y4*Y2;
        break;
    case 37:
        d = 6. * X4 * X * Y - 20. * X3 * Y3 + 6. * X * Y4*Y;
        break;
    case 38:
        d = -6. * X4 * X + 60. * X3 * Y2 - 30. * X * Y4 + 7. * X4 * X * R2 - 70. * X3 * Y2 * R2 +
            35. * X * Y4 * R2;
        break;
    case 39: // Spherical 5
        d = 1. + R2* (-42. + R2 * (420. + R2 *(-1680. + R2 * (3150.+ R2 * (-2772. + 924. * R2)))));
        break;
    case 40:	// spherical 6
        {

        d = -1. + R2 * (56. +R2 * (-756. + R2 * (4200. + R2 * ( -11550.  + R2 * (16632. +R2 * (-12012.  +3432.* R2))))));
        }
        break;

    case 41:	// spherical 7

        d = 1. + R2 * (-72. + R2 * (1260. +R2*(-9240. + R2 * (34650. +R2 * (-72072. + R2 * (84084.
            +R2 * (-51480. + 12870. * R2)))))));
        //d = zpr(8,0,R);
        break;

    default:
        return(0.);
    }
    return d;


}


/*
Public Function Wavefront(x1 As Double, y1 As Double, Order As Integer)
'computes the wavefront deviation from all selected Zernikes
*/


zernikeProcess *zernikeProcess::m_Instance = NULL;
zernikeProcess *zernikeProcess::get_Instance(){
    if (m_Instance == NULL){
        m_Instance = new zernikeProcess();
    }
    return m_Instance;
}

zernikeProcess::zernikeProcess(QObject *parent) :
    QObject(parent),m_dirty_zerns(true)
{
    md = mirrorDlg::get_Instance();;
}

void zernikeProcess::unwrap_to_zernikes(zern_generator *zg, cv::Mat wf, cv::Mat mask){
    // given a wavefront generate arbitrary number of zernikes from it.  Then create wavefront from the zernikies.
    double *m = (double *)(wf.data);

    int size = wf.cols;

    //'calculate LSF matrix elements
    int terms = zg->get_terms_cnt();
    int am_size = terms* terms;
    double* Am = new double[am_size];
    double* Bm = new double[terms];
    for (int  i = 0; i < am_size; ++i)
    {
        Am[i] = 0.;
    }
    for (int i = 0; i < terms; ++i)
        Bm[i] = 0;

    //calculate LSF right hand side
    for(int y = 0; y < size; ++y) //for each point on the surface
    {
        for(int x = 0; x < size; ++x)
        {
             if (mask.at<bool>(x,y))
            {
                int sndx = x + y* size;

                for (int  i = 0; i < terms; ++i)
                {
                    double ipoly = zg->get_zpoly(i,x,y);
                    int dy = i * terms;
                    for (int j = 0; j < terms; ++j)
                    {
                        int ndx = j + dy;
                        Am[ndx] = Am[ndx] +
                                ipoly * zg->get_zpoly(j, x, y);
                    }
                    Bm[i] = Bm[i] + m[sndx] * ipoly;

                }

            }
        }

    }
    // compute coefficients
    gauss_jordan (terms, Am, Bm);
    for (int i = 0; i < terms; ++i){
        qDebug() << i << " " << Bm[i];
    }
    zg->set_zcoefs(Bm);
    delete[] Am;
}
// compute zernikes from unwrapped surface
#define SAMPLE_WIDTH 2
double zernikeProcess::unwrap_to_zernikes(wavefront &wf)
{
    int nx = wf.data.cols;
    int ny = wf.data.rows;

    cv::Mat surface = wf.data;

    static double RMS = 0.;
    if (!m_dirty_zerns)
        return RMS;

    Z = cv::Mat(Z_TERMS,1,CV_64F, 0.);

/*
'calculate LSF matrix elements
*/
    int am_size = Z_TERMS * Z_TERMS;
    double* Am = new double[am_size];
    double* Bm = new double[Z_TERMS];
    for (int i = 0; i < am_size; ++i)
    {
        Am[i] = 0.;
    }
    for (int i = 0; i < Z_TERMS; ++i)
        Bm[i] = 0;

    //calculate LSF right hand side
    int step = SAMPLE_WIDTH;

    while ((nx/step) > 100)
    {
        ++step;
    }

    double delta = 1./(wf.m_outside.m_radius);

    for(int y = 0; y < ny; y += step) //for each point on the surface
    {
        //((MainWindow*)parent())->progBar->setValue(100 * y/ny);
        for(int x = 0; x < nx; x += step)
        {
            double ux = (x -wf.m_outside.m_center.x()) * delta;
            double uy = (y -wf.m_outside.m_center.y()) * delta;
            double rho = sqrt(ux * ux + uy * uy);

            if (wf.workMask.at<bool>(y,x) and rho <= 1.)
            {

                for ( int i = 0; i < Z_TERMS; ++i)
                {

                    int dy = i * Z_TERMS;
                    for (int j = 0; j < Z_TERMS; ++j)
                    {
                        int ndx = j + dy;
                        Am[ndx] = Am[ndx] +
                            Zernike(i, ux, uy) * Zernike(j, ux, uy);

                    }

                    // FN is the OPD at (Xn,Yn)
                    Bm[i] = Bm[i] + surface.at<double>(y,x) * Zernike(i, ux, uy);

                }

            }
        }
    }

    // compute coefficients
    gauss_jordan (Z_TERMS, Am, Bm);


    wf.InputZerns.assign(Bm, Bm+Z_TERMS);


    //m_fringe_rms = RMS;
    delete[] Am;
    delete[] Bm;

    return RMS;
}

cv::Mat zernikeProcess::null_unwrapped(wavefront&wf, std::vector<double> zerns, std::vector<bool> enables,
                                       int start_term, int last_term)
{

    int nx = wf.data.cols;
    int ny = wf.data.rows;

    cv::Mat unwrapped = wf.data.clone();


    double scz8 = md->z8 * md->cc;
    if (!md->doNull)
        scz8 = 0.;
    double midx = wf.m_outside.m_center.rx();
    double midy = wf.m_outside.m_center.ry();
    double rad = wf.m_outside.m_radius;

    cv::Mat nulled(ny,nx,CV_64F,0.);
    double maxrho = 0.;
    bool doDefocus = surfaceAnalysisTools::get_Instance()->m_useDefocus;
    double defocus = 0;
    if (doDefocus)
        defocus = surfaceAnalysisTools::get_Instance()->m_defocus;
    for(int  y = 0; y < ny; ++y)
    {
        for(int x = 0; x < nx; ++x)
        {
            if (wf.mask.at<bool>(y,x))
            {

                double ux = (double)(x - midx)/rad;
                double uy = (double)(y - midy)/rad;
                double rho = sqrt(ux * ux + uy * uy);
                maxrho = max(maxrho, rho);
                if (rho >= 1.){
                    continue;
                }

                double sz = unwrapped.at<double>(y,x);

                if (last_term > 7)
                {
                    if (md->doNull && enables[8])
                        sz -= scz8 * Zernike(8,ux,uy);
                }

                for (int z = start_term; z < last_term; ++z)
                {
                    if ((z == 3) & doDefocus)
                        sz -= defocus * Zernike(z,ux,uy);

                    if (!enables[z])
                        sz -= zerns[z] * Zernike(z,ux,uy);
                }
                nulled.at<double>(y,x) =sz;
//                if (nulled.at<double>(y,x) != 0)
//                    qDebug() << "xxx" << x << " yyy " << y << " v "<< nulled.at<double>(y,x);
            }


        }
    }

    //generate_image_from_doubles(nulled_image, nx,ny, CString(L"Nulled"),true);

    return nulled;
}

/*
Public Function Wavefront(x1 As Double, y1 As Double, Order As Integer)
'computes the wavefront deviation from all selected Zernikes
*/
/*
double zernikeProcess::Wavefront(double x1, double y1, int Order)
{
    double S1;


    S1 = 0;

    for(int  j = 0; j < Order; ++j)
    {
        if (zernEnables[j])
        {
            double W = Zernike(j,x1,y1);
            double z = zNulls[j];

//            if (j ==4 && m_remove_bath_astig)
//            {
//                z = z - m_bath_astig;
//            }

            S1 = S1 + z * W;

            if (j == 8 && md->doNull)
            {
                S1 = S1 - md->z8 * md->cc * W;
            }



        }
    }
    return(S1);

}
*/
#define TSIZE 450	// number of points in zern generator
void ZernikeSmooth(cv::Mat wf, cv::Mat mask)
{
    // given a wavefront generate arbitrary number of zernikes from it.  Then create wavefront from the zernikies.
    double *m = (double *)(wf.data);


    int size = wf.cols;
    int step = (double)size/200.;	// 200 samples across the wavefront
    double delta = (double)TSIZE/size;	// scale factor from wavefront to zernike generator

    double* tmp = new double[TSIZE * TSIZE];
    memset(tmp, 0,sizeof(double) * TSIZE * TSIZE);

    zern_generator zg(TSIZE);
    zg.set_spec(6);
    zg.zpoly_list();



    //'calculate LSF matrix elements

    int terms = zg.get_terms_cnt();
    int am_size = terms* terms;
    double* Am = new double[am_size];
    double* Bm = new double[terms];
    for (int  i = 0; i < am_size; ++i)
    {
        Am[i] = 0.;
    }
    for (int i = 0; i < terms; ++i)
        Bm[i] = 0;

    //calculate LSF right hand side
    for(int y = 0; y < size; y += step) //for each point on the surface
    {
        for(int x = 0; x < size; x += step)
        {
            int xx = x* delta;
            int yy = y * delta;
            if (mask.at<bool>(x,y))
            {
                int sndx = x + y* size;

                tmp[xx+yy * TSIZE] = m[sndx];
                for (int  i = 0; i < terms; ++i)
                {
                    double ipoly = zg.get_zpoly(i,xx,yy);
                    int dy = i * terms;
                    for (int j = 0; j < terms; ++j)
                    {
                        int ndx = j + dy;
                        Am[ndx] = Am[ndx] +
                                ipoly * zg.get_zpoly(j, xx, yy);
                    }
                    Bm[i] = Bm[i] + m[sndx] * ipoly;

                }

            }
        }

    }
    // compute coefficients
    gauss_jordan (terms, Am, Bm);

    zg.set_zcoefs(Bm);

    for (int i = 0; i < terms; ++i){
        qDebug() << i << " " << Bm[i];
    }
    delete[] Am;
    delete[] Bm;
    delete[] tmp;
}