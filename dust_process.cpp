#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <math.h>
#include <unordered_map>
#include <map>

using namespace std;

//dinh dang cac bien 
struct dustdata {
    int id;
    tm time;
    double value;
    
};

//chuyen string time sang time
tm timer(const string &strtime) {
    tm timestruct= {};
    stringstream ss(strtime);
    ss >> get_time(&timestruct, "%Y:%m:%d %H:%M:%S");
    return timestruct;
}
//doc data file csv
void data(const string &line, int &id, tm &time, double &value) {
    string tempstring="",strtime="";
    stringstream inputstring(line);

    getline(inputstring, tempstring, ',');
    id = stoi(tempstring);
    
    getline(inputstring, strtime, ',');
    time=timer(strtime); 

    getline(inputstring, tempstring);
    value = stod(tempstring);
}

//kiem tra lap dong 
bool isDuplicate(int id, const tm &time, unordered_map<int, vector<tm>> &idTimeMap, int &duplicateLine) {
    auto &times = idTimeMap[id];
    for (size_t i = 0; i < times.size(); ++i) {
        if (difftime(mktime((tm *)&times[i]), mktime((tm *)&time)) == 0) {
            duplicateLine = i + 1; 
            return true;
        }
    }
    times.push_back(time);
    return false;
}

//kiem tra value hop le
void checkvalid(ifstream &file, ofstream &outlier,ofstream &logfile, ofstream &valid, vector<dustdata> &properties) {
    string line;
    int linecount=0;
    unordered_map<int, vector<tm>> idTimeMap;
    while (getline(file, line)) {
        linecount++;
        
        if (linecount > 10000) {
        cerr << "Error 06: Input file is too large." << endl;
        break;
        }

        if (line.empty()) {
            logfile << "Error 04: Data is missing at line " << linecount << endl;
            continue;
        }
        
        int id = 0;
        tm time={};
        double value = 0;
        int count=0;

        try {
            data(line, id, time, value);
        } catch (...) {
            logfile << "Error 04: Data is missing at line " << linecount << endl;
            continue;
        }

        int duplicateLine = 0;
        if (isDuplicate(id, time, idTimeMap, duplicateLine)) {
            logfile << "Error 05: Data is duplicated at line " << duplicateLine << " and " << linecount << endl;
            continue;
        }

        if (id<=0){
            logfile<< "Error 04: Data is missing at line " << linecount << endl;  
            continue;
        }

        
        if (value >= 3 && value <= 550.5) {
            valid << line << endl;
            properties.push_back({id, time, value});
        } else {
            count+=1;
            outlier << line << endl;
        }
    }
   
    
}

//tinh gia tri aqi
int dustquality(double &value){
    double aqi[7][4]={{0,12,0,50},{12,35.5,50,100},{35.5,55.5,100,150},{55.5,150.5,150,200},{150.5,250.5,200,300},{250.5,350.5,300,400},{350.5,550.5,400,500}};
    int aqivalue=0;
    for (int i=0;i<7;i++){
        if (value > aqi[i][0] && value <= aqi[i][1]){
            aqivalue=round((aqi[i][3]-aqi[i][2])/(aqi[i][1]-aqi[i][0])*(value-aqi[i][0])+aqi[i][2]);
            break;
        }
    }
    return aqivalue;
}

//chat luong cua aqi
string quality(int aqivalue){
    double aqi[7][2]={{0,50},{50,100},{100,150},{150,200},{200,300},{300,400},{400,500}};  
    string temp="";
    string polution[7]={ "Good", "Moderate", "Slightly unhealthy", "Unhealthy", "Very unhealthy", "Hazardous", "Extremely hazardous" };
    for (int i=0;i<7;i++){
        if (aqivalue > aqi[i][0] && aqivalue <= aqi[i][1]){
            temp=polution[i];
            break;
        }
    }
    return temp; 
}

//bien doi gio trung binh +1 hour
string nexthour(tm time){
    time.tm_hour += 1;
    time.tm_min=0;
    time.tm_sec=0;
    mktime(&time);
    stringstream out;
    out << put_time(&time, "%Y:%m:%d %H:%M:%S");
    return out.str();
}

//sap xep theo value
void valuesort(vector<dustdata> &props){
    int n=props.size();
    for (int i = 0; i < n-1;i++){
        for (int j = i+1; j < n;j++){
            if (props[i].value > props[j].value){
                swap(props[i],props[j]);
            }   
        }
    }
}

// sap xep theo idid
void idsort(vector<dustdata> &props){
    int n=props.size();
    for (int i = 0; i < n-1;i++){
        for (int j = i+1; j < n;j++){
            if (props[i].id > props[j].id){
                swap(props[i],props[j]);
            }   
        }
    }
}

//tinh gia tri trung binhbinh
double mean(vector<dustdata> &props){
    int n=props.size();
    double sum=0;
    for (int i = 0; i < n;i++){
        sum+=props[i].value;
    }
    return sum/n;
}

//tinh trung vivi
double median(vector<dustdata> &props) {
    int n = props.size();
    if (n == 0) return 0;
    if (n % 2 == 1) { 
        return props[n / 2].value;
    } else { 
        return (props[n / 2 - 1].value + props[n / 2].value) / 2.0;
    }
}

//tinh khoang thoi gian 

string timediff(const std::tm &start_time_t, const std::tm &end_time_t) {

    std::time_t start_time = std::mktime(const_cast<std::tm *>(&start_time_t));
    std::time_t end_time = std::mktime(const_cast<std::tm *>(&end_time_t));

    double secdiff = std::difftime(end_time, start_time);

    int hours = static_cast<int>(secdiff) / 3600;
    int minutes = (static_cast<int>(secdiff) % 3600) / 60;
    int seconds = static_cast<int>(secdiff) % 60;

    std::ostringstream result;
    result << setfill('0') << setw(2) << hours << ":"
           << setfill('0') << setw(2) << minutes << ":"
           << setfill('0') << setw(2) << seconds;

    return result.str();
}

//tim gia tri min max mean median
void minmax(vector<dustdata> &properties, ofstream &sumfile) {
    vector<dustdata> temp_data;  
    vector<dustdata> tempprop=properties;
    idsort(tempprop);
    int current_id = tempprop[0].id;
    tm start=properties[0].time;
    tm end=properties.back().time;

    for (const auto &prop : tempprop) {
        if (prop.id == current_id) {
            temp_data.push_back(prop);
        } else {
            
                valuesort(temp_data); 
                double mean_value = mean(temp_data);
                double median_value = median(temp_data);
 
                sumfile << current_id << ",max," << put_time(&temp_data.back().time, "%Y:%m:%d %H:%M:%S") << "," << temp_data.back().value << endl;
                sumfile << current_id << ",min," << put_time(&temp_data[0].time, "%Y:%m:%d %H:%M:%S") << "," << temp_data[0].value << endl;
                sumfile << current_id << ",mean,"  << timediff(start,end)<<"," << fixed << setprecision(2) << mean_value << endl;
                sumfile << current_id << ",median,"  <<timediff(start,end)<< "," << fixed << setprecision(1) << median_value << endl;
            
            temp_data.clear();
            current_id = prop.id;
            temp_data.push_back(prop);
        }
    }

    if (!temp_data.empty()) {
        valuesort(temp_data);
        double mean_value = mean(temp_data);
        double median_value = median(temp_data);

        sumfile << current_id << ",max," << put_time(&temp_data.back().time, "%Y:%m:%d %H:%M:%S") << "," << temp_data.back().value << endl;
        sumfile << current_id << ",min," << put_time(&temp_data[0].time, "%Y:%m:%d %H:%M:%S") << "," << temp_data[0].value << endl;
        sumfile << current_id << ",mean,"  << "," << timediff(start,end)<<","<<fixed << setprecision(2) << mean_value << endl;
        sumfile << current_id << ",median,"  << "," << timediff(start,end)<<","<<fixed << setprecision(1) << median_value << endl;
    }
}

//id max 
int idmax(vector<dustdata> props){
    int n=props.size();
    int max=props[0].id;
    for (int i=0;i<n;i++){
        max=(max > props[i].id) ? max : props[i].id;
    }
    return max;
}

//kiem tra file co ton tai hay khong 
bool fileExists(const string &filename) {
    ifstream file(filename);
    return file.good();
}
//kiem tra header file
bool validateCSVHeader(const string &header) {
    return header == "id,time,value";
}


int main(int argc, char *argv[]) {
    //doc ten file neu ko co de mac dinh
    string inputfilename;
    if (argc > 1) {
        inputfilename = argv[1];
    } else {
        inputfilename = "dust_sensor.csv";
        cout << "Bạn chưa nhập tên file, tự động sử dụng file mặc định: " << inputfilename << endl;
    }
    if (!fileExists(inputfilename)) {
        cerr << "Error 01: File not found or cannot be accessed." << endl;
        return 1;
    }

    //khai bao file xuat 
    ofstream outlierfile("dust_outlier.csv");
    ofstream validfile("dust_valid.csv");
    ofstream aqifile("dust_aqi.csv");
    ifstream file(inputfilename);
    ofstream sumfile("dust_summary.csv");
    ofstream statfile("dust_statistics.csv");
    ofstream logfile("error_log.txt");
    
    vector<dustdata> validData;
    int lineCount = 0;

    //kiem tra co mo duoc file khong
    if (!file) {
        cerr << "Error 01: File not found or cannot be accessed." << inputfilename << endl;
        return 1;
    }

    //header cua file xuat
    string line;
    getline(file, line);
    map<string, int> lineMap;
    //kiem tra header file
    if (!validateCSVHeader(line)) {
        cerr << "Error 02: Invalid CSV file." << endl;
        return 1;
    }
    

    string header = line;
    outlierfile << header << endl;
    validfile << header << endl;
    aqifile<<"id,time,value,aqi,pollution"<<endl;
    sumfile<<"id,parameter,time,value"<<endl;
    statfile<<"id,parameter,time,value"<<endl;

    //task 2.1
    vector<dustdata> properties;
    
    checkvalid(file, outlierfile,logfile, validfile, properties);
    minmax(properties,sumfile);

    //tinh gia tri trung binh va tinh aqi
    vector<double> arrv(100, 0.0); //tong gia tri value tung id  
    vector<int> arrc(100, 0);     //so gia tri tung id
    tm timecheck = properties[0].time;
    int m=idmax(properties);
    vector<vector<int>> arr2(m, vector<int>(7, 0));
    string polution[7]={ "Good", "Moderate", "Slightly unhealthy", "Unhealthy", "Very unhealthy", "Hazardous", "Extremely hazardous" };
    
    for (const auto &property : properties) {
        if ((property.time).tm_hour == timecheck.tm_hour) {
            arrv[property.id - 1] += property.value;
            arrc[property.id - 1]++;
        } else {
            for (int i = 0; i < arrv.size(); i++) {
                if (arrc[i] > 0) {
                    arrv[i] /= arrc[i];
                    aqifile <<i+1<<"," <<nexthour(timecheck)<<"," << fixed << setprecision(1)<< arrv[i]<<","<<dustquality(arrv[i])<<","<<quality(dustquality(arrv[i])) << endl;
                    for (int j=0;j<7;j++){
                        if (polution[j]==quality(dustquality(arrv[i]))) {
                            arr2[i][j]+=1;
                        }
                    }               
                }
            }
        
            fill(arrv.begin(), arrv.end(), 0.0);
            fill(arrc.begin(), arrc.end(), 0);
            timecheck = property.time;
        }
    }
    for (int i = 0; i < arrv.size(); i++) {
        if (arrc[i] > 0) {
            arrv[i] /= arrc[i];
            aqifile <<i+1<<"," << nexthour(timecheck)<<"," << fixed << setprecision(1)<< arrv[i]<<","<<dustquality(arrv[i])<<","<<quality(dustquality(arrv[i])) << endl;
            for (int j=0;j<7;j++){
                if (polution[j]==quality(dustquality(arrv[i]))) {
                    arr2[i][j]+=1;
                }
            }
        }
    }
 
    for (int i=0;i<m;i++){
        for (int j = 0; j < 7; j++){
            statfile<<i+1<<","<<polution[j]<<","<<arr2[i][j]<<endl;
        }
    }
    


    //dong file
    file.close();
    outlierfile.close();
    validfile.close();
    aqifile.close();
    sumfile.close();
    statfile.close();
    logfile.close();

    return 0;
}

