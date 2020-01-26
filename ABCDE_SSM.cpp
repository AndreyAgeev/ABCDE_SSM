// ABCDE_SSM.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "pch.h"
#include <iostream>

int main()
{
	srand((unsigned)time(NULL));
	GError *gerror = NULL;
	Posterior posterior;
	initial_init_posterior(&posterior);
	Thetha prev_thetha;
	Thetha curr_thetha;
	int i = 0, t = 0;
	double eps;
	Config config;
	gchar  filename[] = { 'a', '.', 'i', 'n','i',0 };//argv[0]
	read_config_file_abc(&config, filename, &gerror);
	eps = config.eps;
	double * error = new double[N];
	char  *curr_deep_ini_file = new char [10];
	for (i = 0; i < N; i++)
	{
		do
		{
			curr_thetha = generate_vector_param(NORM_WITH_PARAM);
			prepare_deep_file(config, curr_thetha);//this we use default deep file
			create_curr_deep_ini_file(curr_deep_ini_file, config.name_deep_ini_file, i);//now we copy new data from default deep file to new file with new name for test i iteration
			std::cout << "curr_name " << curr_deep_ini_file << std::endl;
			error[i] = Model(SSM, config.name_exe_file, curr_deep_ini_file);
			std::cout << "Not real(just print 0) error_" << i << "= " << error[i] << std::endl;
		} while (error[i] > config.eps);
		posterior.thetha[i] = curr_thetha;
		posterior.w[i] = 1.0 / (double)N;
	}
	delete[]error;
	delete[]posterior.thetha;
	delete[]posterior.w;
	delete[] curr_deep_ini_file;
	return 0;///////////////////////////////////////////////////////////////////////////////////////
	double alpha, psi, sigma_psi;
	for (t = 0; t < config.t; t++)
	{
		for (i = 0; i < N; i++)
		{
			if (rand() < 0.05)////////////////////////
			{
				curr_thetha = mutation(&posterior, i);
			}
			else
			{
				curr_thetha = crossover(&posterior, i);
			}
			prepare_deep_file(config, curr_thetha);
			error[i] = Model(SSM, config.name_exe_file, config.name_deep_ini_file);
			sigma_psi = prior_distribution(EXPON, 0.005);
			psi = prior_distribution(NORM_WITH_PARAM, error[i], sigma_psi);
			alpha = ((prior_distribution(NORM_WITH_PARAM, curr_thetha.n, SIGMA) *  prior_distribution(NORM_WITH_PARAM, curr_thetha.l, SIGMA) *  prior_distribution(NORM_WITH_PARAM, curr_thetha.lambda, SIGMA)) * psi) / ((prior_distribution(NORM_WITH_PARAM, posterior.thetha[i].n, SIGMA) *  prior_distribution(NORM_WITH_PARAM, posterior.thetha[i].l, SIGMA) *  prior_distribution(NORM_WITH_PARAM, posterior.thetha[i].lambda, SIGMA)) * psi);
			if (alpha > 1)
			{
				posterior.thetha[i] = curr_thetha;
				posterior.w[i] = get_new_probabilities(&posterior, curr_thetha);
			}
		}
	}
	delete[]posterior.thetha;
	delete[]posterior.w;
	return 0;
}
