#include <bits/stdc++.h>
#define ll long long
using namespace std;
ll inf = 1e9 + 7;

void sol()
{
    string a, b;
    cin >> a >> b;
    int n = a.size(), m = b.size();
    vector<int> pa(n + 1, 0), pb(m + 1, 0);
    for (int i = 1; i <= n; i++)
    {
        pa[i] = (pa[i - 1] + a[i - 1] - '0') % 10;
    }

    for (int i = 1; i <= m; i++)
    {
        pb[i] = (pb[i - 1] + b[i - 1] - '0') % 10;
    }
    if (pa[n] != pb[m])
    {
        cout << "-1\n";
        return;
    }
    vector<vector<int>>dp(n+1, vector<int>(m+1,0));
    for(int i =1 ;i<=n;i++){
        for(int j = 1; j<=m;j++){
            if(pa[i]==pb[j]){
                dp[i][j]=1+dp[i-1][j-1];
            }else{
                dp[i][j] = max(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    cout<<dp[n][m]<<"\n";
    return;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    ll t;
    cin >> t;
    while (t--)
    {
        sol();
    }
}