struct sum_count {
    sum_count():sum(),count(0){}
    // Point sum;
    vector<double> sum;
    int count;
    void clear() {
        sum.clear();
        count = 0;
    }
    void tally(Point p){
        vector<double>::iterator iter_sum = sum.begin();
        vector<double>::iterator iter_p = p.getValues().begin();
        if (p.getValues().size()>=sum.size())
        {
            for(;iter_p != p.getValues().end();++iter_p)
            {
                *iter_sum += *iter_p;
            }
        }

        ++count;
    }
    vector<double> mean(){
        vector<double>::iterator iter_sum = sum.begin();
        for(;iter_sum != sum.end();++iter_sum){
            *iter_sum /= count;
        }
        return sum;
    }
    void operator+=(sum_count& other){
        vector<double>::iterator iter_sum = sum.begin();
        vector<double>::iterator iter_other = other.sum.begin();
        if (other.sum.size()>=sum.size())
        {
            for(;iter_other != other.sum.end();++iter_other)
            {
                *iter_sum += *iter_other;
            }
        }
        count += other.count;
    }
};