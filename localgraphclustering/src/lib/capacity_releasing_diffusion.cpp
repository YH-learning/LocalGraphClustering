#ifdef CRD_H

#include "include/routines.hpp"
#include <math.h>

using namespace std;

template<typename vtype, typename itype>
void reset(unordered_map<vtype,double>& f_v, unordered_map<vtype,double>& ex, 
	unordered_map<vtype,double>& cond_temp, unordered_map<vtype,vtype>& l, 
	unordered_map<vtype,vector<vtype>>& labels_temp) {
	f_v.clear();
	ex.clear();
	cond_temp.clear();
	l.clear();
	labels_temp.clear();
}

template<typename vtype, typename itype>
void map_copy(unordered_map<vtype,double>& cond_best_array, unordered_map<vtype,double>& cond_temp)
{
    cond_best_array.clear();
    for (auto iter = cond_temp.begin(); iter != cond_temp.end(); ++iter) {
        cond_best_array[iter->first] = iter->second;
    }
}

template<typename vtype, typename itype>
void mat_vec_dot(itype* ai, vtype* aj, vtype n, unordered_map<vtype,double>& vec, 
	unordered_map<vtype,double>& result)
{
	for (vtype i = 0; i < n; i ++) {
		double sum = 0;
		for (vtype j = ai[i]; j < ai[i+1]; j ++) {
			if (vec.count(aj[j])) {
				sum += vec[aj[j]];
			}
		}
		if (sum != 0) {
			result[i] = sum;
		}
	}
}

template<typename vtype, typename itype>
double vec_vec_dot(unordered_map<vtype,double>& vec1, unordered_map<vtype,double>& vec2)
{
	double result = 0;
	if (vec1.size() < vec2.size()) {
		for (auto iter = vec1.begin(); iter != vec1.end(); ++iter) {
		    if (vec2.count(iter->first)) {
			    result += iter->second*vec2[iter->first];
		    }
	    }
	}
	else {
		for (auto iter = vec2.begin(); iter != vec2.end(); ++iter) {
		    if (vec1.count(iter->first)) {
			    result += iter->second*vec1[iter->first];
		    }
	    }
	}
	
	return result;
}

template<typename vtype, typename itype>
void vec_vec_add(unordered_map<vtype,double>& vec1, unordered_map<vtype,double>& vec2)
{
    for (auto iter = vec2.begin(); iter != vec2.end(); ++iter) {
        if (vec1.count(iter->first) == 0) {
            vec1[iter->first] = iter->second;
        }
        else{
            vec1[iter->first] += iter->second;
        }
    }
}


template<typename T1, typename T2>
void print_map(unordered_map<T1,T2>& m)
{
    cout << m.size() << endl;
    for (auto iter = m.begin(); iter != m.end(); ++iter) {
        cout << iter->first << ": " << iter->second << ", ";
    }
    cout << endl;
}



template<typename vtype, typename itype>
vtype graph<vtype,itype>::capacity_releasing_diffusion(vector<vtype>& ref_node,
    vtype U,vtype h,vtype w,vtype iterations,vtype* cut)
{
	unordered_map<vtype,double> Delta;
	unordered_map<vtype,double> Delta0;
	for (auto iter = ref_node.begin(); iter != ref_node.end(); ++iter) {
        double degree_val = get_degree_unweighted(*iter);
		Delta[*iter] = 2*degree_val;
		Delta0[*iter] = 2*degree_val;
	}

    double cond_best = 100;
    
    unordered_map<vtype,double> f_v,ex,cond_temp,cond_best_array;
    unordered_map<vtype,vtype> l;
    unordered_map<vtype,vector<vtype>> labels_temp,labels;

    for (size_t i = 0; i < iterations; i ++) {
        cout << "iter: " << i << endl;
    	reset<vtype,itype>(f_v,ex,cond_temp,l,labels_temp);
        //print_map<vtype,double>(Delta);
    	unit_flow(Delta, U, h, w, f_v, ex, l);
        //cout << "l size: " << l.size() << endl;
        //print_map<vtype,vtype>(l);
        //print_map<vtype,double>(f_v);
        //print_map<vtype,double>(ex);
    	round_unit_flow(l,cond_temp,labels_temp);

        vtype idx;
        double value = -1;
        for (auto iter = cond_temp.begin(); iter != cond_temp.end(); ++iter) {
        	if (iter->second < value || value < 0) {
        		value = iter->second;
        		idx = iter->first;
        	}
        }
        cout << cond_temp.size() << " " << cond_best << " " << idx << endl;
        if (cond_temp[idx] < cond_best) {
        	cond_best = cond_temp[idx];
        	map_copy<vtype,itype>(cond_best_array, cond_temp);
        	labels = labels_temp;
        }
        double total_excess = 0;
        for (auto iter = f_v.begin(); iter != f_v.end(); ++iter) {
        	if (ex.count(iter->first) == 0) {
        			ex[iter->first] = 0;
        	}
        	total_excess += ex[iter->first];
        	Delta[iter->first] = w*(f_v[iter->first]-ex[iter->first]);
        }
        double degree_val = get_degree_unweighted(ref_node[0]);
        if (total_excess > (degree_val*pow(2,i)/10)) {
        	cout << "Too much excess." << endl;
        	cout << "iteration:" << endl;
        	cout << i << endl;
        	break;
        }
        double sum_ = 0;
        for (auto iter = f_v.begin(); iter != f_v.end(); ++iter) {
        	degree_val = get_degree_unweighted(iter->first);
        	if (iter->second >= degree_val) {
        		sum_ += degree_val;
        	}
        }
        if (sum_ > volume/3) {
        	cout << "Too much flow." << endl;
        	cout << "iteration:" << endl;
        	cout << i << endl;
        	break;
        }
    }
    vtype length = 0;
    vtype idx;
    double value = -1;
    vector<vtype> keys;
    //cout << cond_best_array.size() << endl;
    for (auto iter = cond_best_array.begin(); iter != cond_best_array.end(); ++iter) {
        if (iter->second < value || value < 0) {	
        	value = iter->second;
        	idx = iter->first;
        }
        keys.push_back(iter->first);
    }
    cout << cond_best_array.size() << " " << idx << endl;
    sort(keys.begin(),keys.end(),greater<vtype>());
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
    	if (*iter >= idx) {
    		for (auto iter1 = labels[*iter].begin(); iter1 != labels[*iter].end(); ++iter1) {
    			cut[length++] = *iter1;
    		}
    	}
    }
    return length;
}


template<typename vtype, typename itype>
void graph<vtype,itype>::round_unit_flow(unordered_map<vtype,vtype>& l,
	unordered_map<vtype,double>& cond,unordered_map<vtype,vector<vtype>>& labels)
{
	vector<vtype> keys;
	for (auto iter = l.begin(); iter != l.end(); ++iter) {
		if (labels.count(iter->second) == 0) {
			labels.insert(make_pair(iter->second, vector<vtype> (1,iter->first)));
		}
		else {
			labels[iter->second].push_back(iter->first);
		}
	}
	for (auto iter = labels.begin(); iter != labels.end(); ++iter) {
		keys.push_back(iter->first);
	}
    
	unordered_map<vtype,double> cut,vol,temp_prev,A_temp_prev,temp_new,result;
	double vol_sum = 0;
    double quad_prev = 0;
    sort(keys.begin(),keys.end(),greater<vtype>());
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
    	temp_new.clear();
    	if (*iter == 0) {
    		continue;
    	} 
    	else {
    		for (auto iter1 = labels[*iter].begin(); iter1 != labels[*iter].end(); ++iter1) {
    			temp_new[*iter1] = 1;
    			vol_sum += get_degree_unweighted(*iter1);
    		}
    		vol[*iter] = vol_sum;
    		result.clear();
    		mat_vec_dot<vtype,itype>(ai,aj,n,temp_new,result);
    		double quad_new = vec_vec_dot<vtype,itype>(temp_new,result);
    		double quad_prev_new = vec_vec_dot<vtype,itype>(A_temp_prev,temp_new);
    		cut[*iter] = vol_sum-quad_prev-quad_new-2*quad_prev_new;
    		quad_prev += quad_new+2*quad_prev_new;
    		vec_vec_add<vtype,itype>(temp_prev,temp_new);
    		result.clear();
    		mat_vec_dot<vtype,itype>(ai,aj,n,temp_new,result);
    		for (auto iter1 = result.begin(); iter1 != result.end(); ++iter1) {
    			if (A_temp_prev.count(iter1->first) == 0) {
    				A_temp_prev[iter1->first] = 0;
    			}
    			A_temp_prev[iter1->first] += iter1->second;
    		}
    	}
    }
    for (auto iter = cut.begin(); iter != cut.end(); ++iter) {
    	vtype i = iter->first;
    	double _a = vol[i];
    	double _b = (volume - _a);
    	double demominator = _a < _b ? _a : _b;
    	cond[i] = (iter->second)/demominator;
        //cout << cond[i] << endl;
    }
}

#endif