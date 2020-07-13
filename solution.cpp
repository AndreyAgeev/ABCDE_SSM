#include "pch.h"


Solution::Solution(const Abcde& _main_model, const Deep& _aux_model, const Parametrs& _param) {
	main_model = _main_model;
	aux_model = _aux_model;
	param = _param;
	error = 0.0;
	alpha = 0.0;
}
inline void Solution::copy_posterior(Distribution::Posterior &posterior_to,Distribution::Posterior& posterior_from)
{
/*	Distribution::Thetha * tmp_thetha = posterior_to.thetha;
	double* tmp_w = posterior_to.w;
	posterior_to.thetha = posterior_from.thetha;
	posterior_from.thetha = tmp_thetha;
	posterior_to.w = posterior_from.w;
	posterior_from.w = tmp_w;*/
	for (int i = 0; i < main_model.count_iter; i++)
	{
		posterior_to.thetha[i] = posterior_from.thetha[i];
		posterior_to.w[i] = posterior_from.w[i];
		posterior_to.error[i] = posterior_from.error[i];
	}
}

void Solution::run_manager()
{
	manager.read_log_file(main_model.posterior, main_model.new_posterior, main_model.count_iter, main_model.count_opt_param);
	switch (manager.state)
	{
	    case Run_manager::STATE::INIT:
		    run_init(manager.iter, manager.index_thetha);
		    break;
		case Run_manager::STATE::RUN_APPROXIMATE:
			run_approximate(manager.iter, manager.index_thetha);
			break;
	    case Run_manager::STATE::RUN:
		        run(manager.iter, manager.index_thetha);
			break;
	}
}

void Solution::run_init(int iter, int index_thetha)
{
	ofstream out("log_iteration.txt", std::ios::app);
	out << "RUN_INIT" << endl;
	for (int i = index_thetha; i < main_model.count_iter; i++)
	{
		main_model.curr_thetha = main_model.bounds(main_model.generator.generate_vector_param(Distribution::NORM_WITH_PARAM, main_model.count_opt_param, main_model.mean, main_model.std));
		for (int j = 0; j < main_model.count_opt_param; j++)
		{
			out << main_model.curr_thetha.param[j] << endl;
		}
		aux_model.act_with_config_file();
		aux_model.prepare_tmp_deep_ini_file(main_model.curr_thetha, main_model.optimizing_model_exe, main_model.param_opt_model, main_model.dtype);
		error = aux_model.run();
		out << "error = " << error << endl;
		main_model.posterior.thetha[i] = main_model.curr_thetha;
		main_model.posterior.w[i] = 1.0 / main_model.count_iter;
		main_model.posterior.error[i] = error;
		main_model.new_posterior.thetha[i] = main_model.curr_thetha;
		main_model.new_posterior.w[i] = 1.0 / main_model.count_iter;
		main_model.new_posterior.error[i] = error;
		main_model.posterior.thetha[i].delta = main_model.new_posterior.thetha[i].delta = main_model.generator.prior_distribution(Distribution::TYPE_DISTR::EXPON, 0.005);
		manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, -1, i, main_model.count_opt_param);
	}
	print_log(-1);
	manager.state = Run_manager::STATE::RUN_APPROXIMATE;
	manager.change_state(manager.state);

	run_approximate(0, 0);
	out.close();
}

void Solution::run_approximate(int iter, int index_thetha)
{
	ofstream out("log_iteration.txt", std::ios::app);
	out << "RUN_APPROXIMATE" << endl;

	for (int t = iter; t < main_model.start_iter; t++)
	{
		if (index_thetha >= main_model.count_iter)
			break;
		for (int i = index_thetha; i < main_model.count_iter; i++)
		{
			double choice = main_model.generator.prior_distribution(Distribution::TYPE_DISTR::RANDOM, 0.0, 1.0);
			out << "choice var = " << choice << endl;
			if (choice < 0.05)
			{
				out << "mutation: ";
				main_model.curr_thetha = main_model.mutation(i);
			}
			else
			{
				out << "crossover: ";
				main_model.curr_thetha = main_model.crossover(i);
			}
			for (int j = 0; j < main_model.count_opt_param; j++)
			{
				out << main_model.curr_thetha.param[j] << endl;
			}

			aux_model.act_with_config_file();
			aux_model.prepare_tmp_deep_ini_file(main_model.curr_thetha, main_model.optimizing_model_exe, main_model.param_opt_model, main_model.dtype);
			error = aux_model.run();

			out << "error = " << error << endl;
			alpha = main_model.get_statistics(Parametrs::MODE::INIT, main_model.curr_thetha, error, i);

			out << "alpha original = " << alpha << endl;
			alpha = min(1.0, alpha);
			out << "alpha after = " << alpha << endl;
			if (main_model.accept_alpha(alpha))
			{
				out << "accept" << endl;
				main_model.new_posterior.thetha[i] = main_model.curr_thetha;
				main_model.new_posterior.w[i] = main_model.generator.get_new_weight(main_model.posterior.thetha[i], main_model.curr_thetha, main_model.count_opt_param, main_model.mean, main_model.std);//change-make generator private for abcde
				main_model.new_posterior.error[i] = error;
				main_model.normalize_weights();
			}
			else
				out << "not accept" << endl;
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);

		}
		out << "begin_copy" << endl;
		out << "before" << endl;

		for (int i = 0; i < main_model.count_iter; i++)
		{
			for (int k = 0; k < main_model.count_opt_param; k++)
				out << main_model.posterior.thetha[i].param[k] << endl;
		}
		out << "after" << endl;

		copy_posterior(main_model.posterior, main_model.new_posterior);
		for(int i = 0; i < main_model.count_iter; i++)
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);

		for (int i = 0; i < main_model.count_iter; i++)
		{
			for (int k = 0; k < main_model.count_opt_param; k++)
				out << main_model.posterior.thetha[i].param[k] << endl;
		}
		copy_posterior(main_model.new_posterior, main_model.posterior);
		for (int i = 0; i < main_model.count_iter; i++)
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);
		print_log(t);
		out << "done_copy" << endl;

	}
	if (main_model.mode == Parametrs::DELTA_MODE::MEAN)
	{
		double s = 0.0;
		for (int i = 0; i < main_model.count_iter; i++)
		{
			s += main_model.posterior.thetha[i].delta;
		}
		main_model.posterior.delta_one = s / main_model.count_iter;
		main_model.new_posterior.delta_one = main_model.posterior.delta_one;
	}
	else if (main_model.mode == Parametrs::DELTA_MODE::MED)
	{

	}
	manager.state = Run_manager::STATE::RUN;
	manager.change_state(manager.state);
	manager.change_delta(main_model.posterior.delta_one);
	run(0, 0);
	out.close();
}

void Solution::run(int iter, int index_thetha)
{
	ofstream out("log_iteration.txt", std::ios::app);

	out << "RUN" << endl;
	for (int t = iter; t < main_model.t; t++)
	{
		if (index_thetha >= main_model.count_iter)
			break;
		for (int i = index_thetha; i < main_model.count_iter; i++)
		{
			double choice = main_model.generator.prior_distribution(Distribution::TYPE_DISTR::RANDOM, 0.0, 1.0);
			out << "choice var = " << choice << endl;
			if (choice < 0.05)
			{
				out << "mutation: ";
				main_model.curr_thetha = main_model.mutation(i);
			}
			else
			{
				out << "crossover: ";
				main_model.curr_thetha = main_model.crossover(i);
			}
			for (int j = 0; j < main_model.count_opt_param; j++)
			{
				out << main_model.curr_thetha.param[j] << endl;
			}
			aux_model.act_with_config_file();
			aux_model.prepare_tmp_deep_ini_file(main_model.curr_thetha, main_model.optimizing_model_exe, main_model.param_opt_model, main_model.dtype);
			error = aux_model.run();
			out << "error = " << error << endl;

			alpha = main_model.get_statistics(Parametrs::MODE::AUX, main_model.curr_thetha, error, i);
			out << "alpha original = " << alpha << endl;
			alpha = min(1.0, alpha);
			out << "alpha after = " << alpha << endl;
			if (main_model.accept_alpha(alpha))
			{
				out << "accept" << endl;
				main_model.new_posterior.thetha[i] = main_model.curr_thetha;
				main_model.new_posterior.w[i] = main_model.generator.get_new_weight(main_model.posterior.thetha[i], main_model.curr_thetha, main_model.count_opt_param, main_model.mean, main_model.std);//change-make generator private for abcde
				main_model.new_posterior.error[i] = error;
				main_model.normalize_weights();

			}
			else
				out << "not accept" << endl;
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);

		}
		
		out << "begin_copy" << endl;
		out << "before" << endl;

		for (int i = 0; i < main_model.count_iter; i++)
		{
			for (int k = 0; k < main_model.count_opt_param; k++)
				out << main_model.posterior.thetha[i].param[k] << endl;
		}
		out << "after" << endl;
		copy_posterior(main_model.posterior, main_model.new_posterior);
		for (int i = 0; i < main_model.count_iter; i++)
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);
		for (int i = 0; i < main_model.count_iter; i++)
		{
			for (int k = 0; k < main_model.count_opt_param; k++)
				out << main_model.posterior.thetha[i].param[k] << endl;
		}
		copy_posterior(main_model.new_posterior, main_model.posterior);
		for (int i = 0; i < main_model.count_iter; i++)
			manager.create_log_file(manager.state, main_model.posterior, main_model.new_posterior, t, i, main_model.count_opt_param);
                print_log(t);
		out << "done_copy" << endl;

	}
	manager.state = Run_manager::STATE::END;
	manager.change_state(manager.state);
	out.close();
}

void Solution::print_log(int iter)
{
	ofstream logfile("log_result.txt", std::ios::app);
	logfile << "iteration = " << iter << endl;
	int print;
	for (int i = 0; i < main_model.count_iter; i++)
	{
		logfile << "element number = " << i << " ";
		for (int j = 0; j < main_model.count_opt_param; j++)
		{
			if (main_model.dtype[j] == 0)
			{
				print = (int)main_model.posterior.thetha[i].param[j];
				logfile << "param[" << j << "] = " << print << " ";

			}
			else
			    logfile << "param[" << j << "] = " << main_model.posterior.thetha[i].param[j] << " ";
		}
		logfile << "w = " << main_model.posterior.w[i] << " ";
		logfile << "error = " << main_model.posterior.error[i] << " ";

		logfile << endl;
	}
	logfile.close();
}
