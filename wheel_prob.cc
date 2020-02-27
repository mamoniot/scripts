#include "basic.h"
#include "stdio.h"
#include "math.h"

//stardew valley fair wheel calculator

static void do_attempt(float prob_success, bool is_success, float* amount, float* prob, float user_data) {
	float amount_bet = ceilf(user_data*(*amount));
	amount_bet = amount_bet > *amount ? *amount : amount_bet;
	amount_bet = amount_bet < 5.0 ? 5.0 : amount_bet;
	if(is_success) {
		*amount += amount_bet;
		*prob *= prob_success;
	} else {
		*amount -= amount_bet;
		*prob *= 1 - prob_success;
	}

}
static float do_attempts(float prob_success, int trials, float* amount, float* prob, float user_data) {
	if(trials > 0) {
		float expect = 0;
		float success_amount = *amount;
		float failure_amount = *amount;
		float success_prob = *prob;
		float failure_prob = *prob;

		do_attempt(prob_success, 1, &success_amount, &success_prob, user_data);
		if(success_amount > 0) {
			expect += do_attempts(prob_success, trials - 1, &success_amount, &success_prob, user_data);
		}

		do_attempt(prob_success, 0, &failure_amount, &failure_prob, user_data);
		if(failure_amount > 0) {
			expect += do_attempts(prob_success, trials, &failure_amount, &failure_prob, user_data);
		}
		return expect;
	} else {
		// printf("%f, %f\n", *prob, *amount);
		float value = 0;
		if(*amount > 3400) {
			value = 650;
		} else if(*amount > 2800) {
			value = 600;
		} else if(*amount > 2000) {
			value = 500;
		} else if(*amount > 800) {
			value = 50;
		} else if(*amount > 500) {
			value = 2;
		} else if(*amount > 100) {
			value = 1;
		}
		return value*(*prob);
	}
}

static float expected(float prob_success, int trials, float init_amount, float user_data) {
	float prob = 1;
	return do_attempts(prob_success, trials, &init_amount, &prob, user_data);
}


int main() {
	float move_by = .01;
	float cut = .9;
	float pre_cut_amount = 0;
	while(1) {
		float cut_amount = expected(.75, 10, 200, cut);
		printf("%f = %f, %f\n", cut, cut_amount, move_by);
		cut = cut > 0.99 ? 0.99 : cut;
		cut = cut < 0.2 ? 0.2 : cut;
		if(cut_amount > pre_cut_amount) {
			move_by *= 1.1;
		} else if(cut_amount < pre_cut_amount) {
			move_by /= -2;
		};
		cut += move_by;
		pre_cut_amount = cut_amount;
	}

	return 0;
}
