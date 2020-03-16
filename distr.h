#pragma once
#include "pch.h"
//DISTRIBUTION
//#define NORM 0
//#define EXPON 1
//#define NORM_WITH_PARAM 2
//#define RANDOM 8
#define MEAN_ERROR 3
#define LOFFSET log((double)RAND_MAX)
#define MU_N 5
#define MU_L 12
#define MU_LAMBDA 5
#define SIGMA 0.5
#define SSM 7
class Distribution
{
    public:
	    enum TYPE_DISTR
        {
	        NORM,
	        EXPON,
	        NORM_WITH_PARAM,
	        RANDOM
        };
		typedef struct
		{
			int n;
			int l;
			double lambda;
		}Thetha;
		typedef struct
		{
			Thetha * thetha;
			double * w;
		}Posterior;

		double prior_distribution(const int mode, const double param1, const double param2)
		{
			if (mode == NORM)
			{
				return getNormalSample();
			}
			if (mode == NORM_WITH_PARAM)
			{
				return getNormalSampleWithParam(param1, param2);//double x = normalRandom()*sigma+Mi;
			}
			if (mode == EXPON)
			{
				return getLrand(param1);
			}
			if (mode == RANDOM)
			{
				return getRandomSample(param1, param2);
			}
			//return ERROR_MODE;
		}
		double getRandomSample(double param1, double param2)
		{
			std::random_device random_device;
			std::mt19937 generator(random_device());

			std::uniform_int_distribution<> distribution(param1, param2); // ����������� ������������� [10, 20]

			double x = distribution(generator);
			return x;
		}
		double getNormalSample()
		{
			double u = ((double)rand() / (RAND_MAX)) * 2 - 1;
			double v = ((double)rand() / (RAND_MAX)) * 2 - 1;
			double r = u * u + v * v;
			if (r == 0 || r > 1) return getNormalSample();
			double c = sqrt(-2 * log(r) / r);
			return u * c;
		}

		double getNormalSampleWithParam(double mean, double var)//is this gaussian kernel?
		{
			//return var * getNormalSample() + mean;
			std::random_device mch;
			std::default_random_engine gen(mch());
			std::normal_distribution<double> d(mean, var);
			return d(gen);
		}

		double getLrand(double l)
		{
			double u;
			u = rand() / (RAND_MAX + 1.0);
			return -log(1 - u) / l;
		}
		double* ExponentionalDistribution(double param)//C++ FUNCTION TO CHECK - DELETE AFTER CHECK
		{
			/*std::default_random_engine generator;
			std::exponential_distribution<double> distribution(param);*/
		/*	int i = 0;
			double *number = (double*)malloc(sizeof(double) * N);
			//	srand((unsigned)time(NULL));
			for (i; i < N; i++)
			{
				//number[i] = distribution(generator);
				number[i] = getLrand(param);
			}
			return number;
			*/
		}
		double variancy(Posterior *posterior, const int mode)
		{
			/*int i = 0, j = 0;
			double s = 0;
			for (i=0; i < N; i++)
			{
				double in_s = 0;
				for (j = 0; j < N; j++)
				{
					in_s += posterior->thetha[j]/(double)N;
				}
				s += posterior->thetha[i] - in_s;
			}
			return (2.0 * (1.0 / (double)N)) * (s * s);*/
			int i = 0, j = 0;
			int N = 0;
			double s = 0.0;
			for (i = 0; i < N; i++)
			{
				if (mode == 0)
					s += posterior->thetha[i].n;
				if (mode == 1)
					s += posterior->thetha[i].l;
				if (mode == 2)
					s += posterior->thetha[i].lambda;
			}
			s = s / (double)N;
			double var = 0.0;
			for (i = 0; i < N; i++)
			{
				if (mode == 0)
					var += (posterior->thetha[i].n - s) * (posterior->thetha[i].n - s);
				if (mode == 1)
					var += (posterior->thetha[i].l - s) * (posterior->thetha[i].l - s);
				if (mode == 2)
					var += (posterior->thetha[i].lambda - s) * (posterior->thetha[i].lambda - s);
			}
			return var;
		}

		double get_new_probabilities(Posterior * posterior, Thetha thetha)
		{
			int N = 0;

			int j = 0;
			double s = 0;
			double mean, var;
			var = variancy(posterior, 0) + variancy(posterior, 1) + variancy(posterior, 2);
			for (j = 0; j < N; j++)
			{
				mean = (posterior->thetha[j].l + posterior->thetha[j].n + posterior->thetha[j].lambda) / 3;
				s += posterior->w[j] * getNormalSampleWithParam(mean, var);
			}
			mean = (thetha.l + thetha.n + thetha.lambda) / 3;
			return getNormalSampleWithParam(mean, var) / s;
		}

		Thetha& generate_vector_param(TYPE_DISTR mode)
		{
			Thetha thetha;
			thetha.n = (int)prior_distribution(mode, MU_N, SIGMA);
			thetha.l = (int)prior_distribution(mode, MU_L, SIGMA);
			thetha.lambda = prior_distribution(mode, MU_LAMBDA, SIGMA);
			return thetha;
		}


};