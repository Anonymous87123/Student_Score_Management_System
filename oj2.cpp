#include<bits/stdc++.h>
#define int long long
using namespace std;
int cmp(vector<int>&a,vector<int>&b){//传倒过来的
    for(int i=a.size()-1;i>=0;--i){
        if(a[i]>b[i])return 1;
        else if(a[i]<b[i])return -1;
    }
    return 0;
}
vector<int>add(vector<int>&a,vector<int>&b){
    int n=max(a.size(),b.size())+3;//0到n
    vector<int>result(n,0);
    a.resize(n,0);b.resize(n,0);
    for(int i=0;i<n;i++)result[i]=a[i]+b[i];
    for(int i=0;i<n-1;i++){
        while(result[i]>=10){
            result[i]-=10;result[i+1]++;
        }
    }
    while(result.size()>1&&result.back()==0)result.pop_back();
    return result;
}
vector<int>sub(vector<int>&a,vector<int>&b){//a>b默认
    int n=max(a.size(),b.size())+3;//0到n
    a.resize(n,0);b.resize(n,0);
    vector<int>result(n,0);
    if(cmp(a,b)==0){
        while(result.size()>1&&result.back()==0)result.pop_back();
        return result;
    }else{
        for(int i=0;i<n;i++)result[i]=a[i]-b[i];
        for(int i=0;i<n-1;i++){
            while(result[i]<0){
                result[i]+=10;result[i+1]--;
            }
        }
    }
    while(result.size()>1&&result.back()==0)result.pop_back();
    return result;
}
void print(vector<int>& result) { // 此时result已经是倒过来的
    while (result.size() > 1 && result.back() == 0) result.pop_back();
    string res = "";
    for (int i = 0; i <= result.size() - 1; ++i) {
        if (result[i] == 0) res += "0";
        else if (result[i] == 1) res += "1";
        else if (result[i] == 2) res += "2";
        else if (result[i] == 3) res += "3";
        else if (result[i] == 4) res += "4";
        else if (result[i] == 5) res += "5";
        else if (result[i] == 6) res += "6";
        else if (result[i] == 7) res += "7";
        else if (result[i] == 8) res += "8";
        else if (result[i] == 9) res += "9";
        // 修改：在不是最高位且满足每三位时添加逗号
        if (i != result.size() - 1 && (i + 1) % 3 == 0) res += ",";
    }
    for (int i = res.size() - 1; i >= 0; --i) cout << res[i];
    cout << endl;
}
signed main(){
    string s1,s2,op;
    while(cin>>s1>>op>>s2){
    vector<int>num1;vector<int>num2;
    bool neg1=false,neg2=false;
    for(int i=s1.size()-1;i>=0;i--)//倒过来
    {
        char c=s1[i];
        if(c=='0')num1.push_back(0);
        else if(c=='1')num1.push_back(1);
        else if(c=='2')num1.push_back(2);
        else if(c=='3')num1.push_back(3);
        else if(c=='4')num1.push_back(4);
        else if(c=='5')num1.push_back(5);
        else if(c=='6')num1.push_back(6);
        else if(c=='7')num1.push_back(7);
        else if(c=='8')num1.push_back(8);
        else if(c=='9')num1.push_back(9);
        else if(c=='-')neg1=true;
    }
    for(int i=s2.size()-1;i>=0;i--)
    {
        char c=s2[i];
        if(c=='0')num2.push_back(0);
        else if(c=='1')num2.push_back(1);
        else if(c=='2')num2.push_back(2);
        else if(c=='3')num2.push_back(3);
        else if(c=='4')num2.push_back(4);
        else if(c=='5')num2.push_back(5);
        else if(c=='6')num2.push_back(6);
        else if(c=='7')num2.push_back(7);
        else if(c=='8')num2.push_back(8);
        else if(c=='9')num2.push_back(9);
        else if(c=='-')neg2=true;
    }
    int n=max(num1.size(),num2.size())+1;//0到n
    num1.resize(n,0);num2.resize(n,0);
    if(op=="+"){
        if(neg1==true&&neg2==true){//都为负数
            vector<int>res=add(num1,num2);
            cout<<'-';print(res);
        }else if(neg1==false&&neg2==true){//a为正数，b为负数
            if(cmp(num1,num2)==0)cout<<0<<endl;
            else if(cmp(num1,num2)==1){//|a|>|b|
                vector<int>res=sub(num1,num2);
                print(res);
            }else if(cmp(num1,num2)==-1){//|a|<|b|
                vector<int>res=sub(num2,num1);
                cout<<'-';print(res);
            }
        }else if(neg1==true&&neg2==false){//a为负数，b为正数
            if(cmp(num1,num2)==0)cout<<0<<endl;
            else if(cmp(num1,num2)==1){//|a|>|b|
                vector<int>res=sub(num1,num2);
                cout<<'-';print(res);
            }else if(cmp(num1,num2)==-1){//|a|<|b|
                vector<int>res=sub(num2,num1);
                print(res);
            }
        }else if(neg1==false&&neg2==false){//都为正数
            vector<int>res=add(num1,num2);
            print(res);
        }
    }else if(op=="-"){
        if(neg1==true&&neg2==true){//都为负数
            if(cmp(num1,num2)==0)cout<<0<<endl;
            else if(cmp(num1,num2)==1){//|a|>|b|
                vector<int>res=sub(num1,num2);
                cout<<'-';print(res);
            }else if(cmp(num1,num2)==-1){//|a|<|b|
                vector<int>res=sub(num2,num1);
                print(res);
            }
        }else if(neg1==false&&neg2==true){//a为正数，b为负数
            vector<int>res=add(num1,num2);
            print(res);
        }else if(neg1==true&&neg2==false){//a为负数，b为正数
            vector<int>res=add(num1,num2);
            cout<<'-';print(res);
        }else if(neg1==false&&neg2==false){//都为正数
            if(cmp(num1,num2)==0)cout<<0<<endl;
            else if(cmp(num1,num2)==1){//|a|>|b|
                vector<int>res=sub(num1,num2);
                print(res);
            }else if(cmp(num1,num2)==-1){//|a|<|b|
                vector<int>res=sub(num2,num1);
                cout<<'-';print(res);
            }
        }
    }}
 
}