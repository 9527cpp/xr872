#include <stdio.h>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>
#include <functional>
#include <numeric>
#include <chrono>
#include <map>
#include <unordered_map>
using namespace std;
using namespace chrono;
struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};
TreeNode * createBinaryTree(int *arr,int len)
{
    TreeNode *root=NULL;
    TreeNode *q=NULL;
    queue<TreeNode *> qtn;
    if(!len || !arr)return root;
    root = new TreeNode(*arr++);
    qtn.push(root);
    int i = 1;
    int lor = 0;//0 left   1 right
    q = qtn.front();
    while(i < len)
    {
        TreeNode *p=NULL;
        int val = *arr;
        if(val!=0)
        {
            p = new TreeNode(val);
        }
        if(!lor)
        {
            q->left = p;
            if(p)qtn.push(p);
        }
        else
        {
            q->right = p;
            if(p)qtn.push(p);
            qtn.pop();
            q = qtn.front();
        }
        lor = !lor;
        arr++;
        i++;
    }
    return root;
}


TreeNode * createBinarySearchTree(int *arr,int len)
{
    TreeNode *root=NULL;
    TreeNode *q=NULL;
    if(!len || !arr)return root;
    root = new TreeNode(*arr++);
    int i = 1;
    while(i < len)
    {
        TreeNode *p=NULL;
        int val = *arr;
        if(val!=0)
        {
            p = new TreeNode(val);
        }
        else// jump 0
        {
            arr++;
            i++;
            continue;
        }
        q = root;
        while(1)
        {
            if(val < q->val)
            {
                if(!q->left){q->left = p;break;}
                q = q->left;
            }
            else
            {
                if(!q->right){q->right = p;break;}
                q = q->right;
            }
        }
        arr++;
        i++;
    }
    return root;
}

//// BFS queue   DFS stack
class Solution {
public:
    // delete a node from a BST
    TreeNode* deleteNode(TreeNode* root, int key) {
        TreeNode * pt = root;
        TreeNode * p = root;       
        TreeNode * pkey = root;
        // find the key node
        while(pkey->val!=key)
        {
            p = pkey;
            if(pkey->val < key)pkey = pkey->right;
            else if(pkey->val > key)pkey = pkey->left;
        }
        pt = pkey;
        // find the pos
        if(pt->right)
        {
            p = pt;
            pt = pt->right;
            while(pt->left)
            {
                p = pt;
                pt = pt->left; 
            }      
            pt->left = pkey ->left;
            pt->right = pkey->right; 
            p->right = NULL;       
        }
        else if(pt->left)
        {
            p = pt;
            pt = pt->left;
            while(pt->right)
            {
                p = pt;
                pt = pt->right;
            }
            pt->left = pkey ->left;
            pt->right = pkey->right;
            p->left = NULL;
        } 
        return root;
    }

    vector<int> findFrequentTreeSum(TreeNode* root) {
        unordered_map<int,int>mii;
        vector<int>vi;
        if(!root)return vi;
        int sum = 0;
        function<int(TreeNode*)>dfs_lambda=[&dfs_lambda,&mii](TreeNode* root)->int
        {   
            int sum = 0;       
            if(root->left)sum += dfs_lambda(root->left);  
            if(root->right)sum += dfs_lambda(root->right); 
            sum+=root->val;
            mii[sum]++;
            //printf("\r\nroot->val:%d,%d\r\n",root->val,sum);
            return sum;
        };
        sum = dfs_lambda(root);
        int maxele = (*max_element(mii.begin(),mii.end(),[](pair<int,int> a, pair<int,int> b)->bool{return a.second<b.second?true:false;})).second;
        for(auto item:mii)
        {
            if(item.second == maxele)vi.push_back(item.first);
        }
        return vi;
    }

    vector<int> rightSideView(TreeNode* root) {
        vector<int>vi;
        if(!root)return vi;
        int h = 0;
        int hp = 0;
        function<void(TreeNode *)>dfs_right =[&dfs_right,&vi,&h,&hp](TreeNode *root)
        {
            if(!root)return;
            h++;
            if(h>hp)
            {
                hp = h;
                vi.push_back(root->val);
            }
            dfs_right(root->right);
            dfs_right(root->left);
            h--;
        }; 
        dfs_right(root);
        return vi;
    }

    vector<vector<int>> pathSum(TreeNode* root, int sum) {
        vector<vector<int> >vvi;
        if(!root)return vvi;
        int sum_t = 0;
        vector<int>vi;
        function<void(TreeNode *,int)> pathSum_lambda = 
        [&pathSum_lambda,&sum_t,&vi,&vvi](TreeNode* root, int sum)
        {           
            sum_t += root->val; 
            vi.push_back(root->val);
            if(root->left)  pathSum_lambda(root->left,sum);                    
            if(root->right) pathSum_lambda(root->right,sum); 
            if(!root->left && !root->right&&sum_t == sum)
            {
                vvi.push_back(vi);
            }
            vi.pop_back();
            sum_t -= root->val;
        }; 
        pathSum_lambda(root,sum);   
        return vvi;      
    }

    bool isBalanced(TreeNode* root) {
        if(!root)return true;
        bool bBalance = true;
        function<int(TreeNode*,bool&)>height = [&height](TreeNode*root,bool &bBalance)->int
        {
            if(!root)return 0;
            if(!bBalance)return 0;
            int hl = height(root->left,bBalance);
            int hr = height(root->right,bBalance);
            if(hl > hr +1 || hr > hl+1)bBalance = false;
            return max(hl,hr)+1; 
        };
        height(root,bBalance);
        return bBalance;
    }

    int height(TreeNode * root)
    {
        if(!root)return 0;
        int hl = height(root->left);
        int hr = height(root->right);
        return max(hl,hr)+1; 
    }

    int kthSmallest(TreeNode* root, int k) {
        stack<TreeNode *>stn;
        if(!root)return 0;
        TreeNode * p ;
        int kmin = 0;
        int count = 0;
        while(root)
        {
            stn.push(root);
            root = root->left;
        }    
        while(!stn.empty())
        {
            p = stn.top();
            stn.pop();
            count++;
            if(count == k)break;
            if(p->right)
            {
                p = p->right;
                stn.push(p);
                while(p->left)
                {   
                    p = p->left;
                    stn.push(p);
                }
            }
        }
        return p->val;
    }

    int getMinimumDifference(TreeNode* root) {
        stack<TreeNode *> stn;
        vector<int>vi,vit;
        TreeNode * pt = root;
        if(!root) return 0;
        while(pt){stn.push(pt);pt =pt->left;}
        while(!stn.empty())
        {
            TreeNode * pt = stn.top();
            vi.push_back(pt->val);
            stn.pop();
            if(pt->right)
            {
                pt = pt->right;
                while(pt)
                {
                    stn.push(pt);
                    pt = pt->left;            
                }
            }    
        }
        adjacent_difference(vi.begin(),vi.end(),vi.begin());
        int minval =*min_element(vi.begin()+1,vi.end());
        return minval;
    }

    vector<int> findMode(TreeNode* root) {
        vector<int> ans;
        if(root==NULL)return ans;
        function<void(vector<int> &,TreeNode*)> findMode_lambada = [&findMode_lambada](vector<int> &ans,TreeNode *root)
        {
            if(root->left)findMode_lambada(ans,root->left);   
            ans.push_back(root->val);         
            if(root->right)findMode_lambada(ans,root->right);        
        }; 
        findMode_lambada(ans,root);  
        unordered_map<int,int> umii;
        for(auto item:ans)umii[item]++; 
        ans.clear();
        int maxele = (*max_element(umii.begin(),umii.end(),[](pair<int,int> a, pair<int,int> b)->bool{return a.second<b.second?true:false;})).second;
        for(auto item:umii)
        {
            if(item.second == maxele)ans.push_back(item.first);
        }
        return ans;
    }

    int countNodes(TreeNode* root) {
        int sum = 0;
        if(!root)return 0;
        function<int(TreeNode*)> countNodes_lambada = 
        [&countNodes_lambada,&sum](TreeNode *root)->int
        {
            if(root->left)countNodes_lambada(root->left);          
            if(root->right)countNodes_lambada(root->right);  
            sum++;
            return sum;
        };
        sum = countNodes_lambada(root);  
        return sum;
    }

    void recoverTree(TreeNode* root) {
        vector<TreeNode *> vtnp;
        vector<int> vi; 
        if(!root)return;
        function<void(vector<TreeNode *> &,vector<int> &,TreeNode*)> recoverTree_lambada = 
        [&recoverTree_lambada](vector<TreeNode *> &vtnp,vector<int> &vi,TreeNode *root)
        {
            if(root->left)recoverTree_lambada(vtnp,vi,root->left);   
            vtnp.push_back(root); 
            vi.push_back(root->val);        
            if(root->right)recoverTree_lambada(vtnp,vi,root->right);        
        };
        recoverTree_lambada(vtnp,vi,root);  
        sort(vi.begin(),vi.end(),[](int a,int b)->bool{return a<b;});  
        auto itrvtn =vtnp.begin();
        auto itrvi = vi.begin();
        for(;itrvtn!=vtnp.end();itrvtn++,itrvi++)
        {
            (*itrvtn)->val = *itrvi;
        }  
        return;    
    }

    int sumNumbers(TreeNode* root) {
        if(!root)return 0;
        int sum_t = 0,sum = 0;
        function<int(TreeNode *)> sumNumbers_lambda = 
        [&sumNumbers_lambda,&sum_t,&sum](TreeNode* root)->int
        {    
            if(!root)return sum;
            sum_t *=10;       
            sum_t += root->val; 
            if(root->left)  sumNumbers_lambda(root->left);                    
            if(root->right) sumNumbers_lambda(root->right); 
            if(!root->left && !root->right)
            {
                sum += sum_t;
            }
            sum_t -= root->val;
            sum_t/=10;
            return sum;
        };   
        return sumNumbers_lambda(root);   
    }

    bool hasPathSum(TreeNode* root, int sum) {
        if(!root)return false;
        int sum_t = 0;
        bool bfind = false;
        function<bool(TreeNode *,int)> hasPathSum_lambda = 
        [&hasPathSum_lambda,&sum_t,&bfind](TreeNode* root, int sum)->bool
        {    
            if(bfind)return true;       
            sum_t += root->val; 
            if(root->left)  hasPathSum_lambda(root->left,sum);                    
            if(root->right) hasPathSum_lambda(root->right,sum); 
            if(!root->left && !root->right)
            {
                if(sum_t == sum)
                bfind = true;
            }
            sum_t -= root->val;
            return bfind;
        };   
        return hasPathSum_lambda(root,sum);
    }

    TreeNode* lowestCommonAncestor(TreeNode* root, TreeNode* p, TreeNode* q) {
        stack<TreeNode *> stnp,stnq;
        TreeNode * pA = NULL;
        if(root==NULL)return pA;
        int findp = 0;
        int findq = 0;
        function<void(TreeNode*,TreeNode*,TreeNode*,stack<TreeNode *>&,stack<TreeNode *>&)> lowestCommonAncestor_lambada = 
        [&lowestCommonAncestor_lambada,&findp,&findq,&stnp,&stnq]
        (TreeNode *root,TreeNode* p, TreeNode* q,stack<TreeNode *>&stnp,stack<TreeNode *>&stnq)
        {
            if(!findp)stnp.push(root);
            if(!findq)stnq.push(root);
            if(root->val == p->val)findp = 1;
            if(root->val == q->val)findq = 1;
            if(findp && findq) return;
            if(root->left)lowestCommonAncestor_lambada(root->left,p,q,stnp,stnq);                    
            if(root->right)lowestCommonAncestor_lambada(root->right,p,q,stnp,stnq);  
            if(!findp)stnp.pop(); 
            if(!findq)stnq.pop(); 
        }; 
        lowestCommonAncestor_lambada(root,p,q,stnp,stnq); 
        while(stnp.size()!=stnq.size())
        {
            if(stnp.size()>stnq.size())
            {
                stnp.pop();
            }
            else stnq.pop();
        }
        while(!stnp.empty() &&!stnq.empty())
        {
            TreeNode * p = stnp.top();
            TreeNode * q = stnq.top();
            if(p->val == q->val)
            {
                pA = p;
                break;
            }
            stnp.pop();
            stnq.pop();
        }
        return pA;   
    }

    TreeNode* lowestCommonAncestorBST(TreeNode* root, TreeNode* p, TreeNode* q) {
        TreeNode *parent = root;
        while(parent)
        {
            if((p->val <= parent->val && q->val >= parent->val)
              ||(p->val >= parent->val && q->val <= parent->val))break;
            else if(p->val < parent->val && q->val < parent->val)
            {
                parent = parent->left;
            }
            else if(p->val > parent->val && q->val > parent->val)
            {
                parent = parent->right;
            }
        }
        return parent;
    }
    vector<string> binaryTreePaths(TreeNode* root) {
        vector<string> vs;
        stack<TreeNode *>stn;
        stack<string>ss;
        stringstream sstream;
        if(!root)return vs;
        TreeNode * p = root;
        TreeNode *lastp = NULL;
        sstream<<p->val;
        string s = string(sstream.str() );
        stn.push(p);
        ss.push(s);
        while(p->left)
        {
            stn.push(p->left);
            sstream.str("");
            sstream<<p->left->val;
            ss.push(string(ss.top()+"->"+string(sstream.str())));
            p = p->left;
        }
        while(!stn.empty())
        {
            TreeNode *p = stn.top();
            if(p->right)
            {
                if(lastp && p->right == lastp)
                {
                    lastp = p;
                    ss.pop();
                    stn.pop();
                    continue;
                }
                p = p->right;
                while(p)
                {
                    stn.push(p);
                    sstream.str("");
                    sstream<<p->val;
                    ss.push(string(ss.top()+"->"+string(sstream.str())));
                    p = p->left;
                }
            }
            else
            {
                lastp = p;
                if(!p->left)
                vs.push_back(ss.top());
                ss.pop();
                stn.pop();
            }
        }
        return vs;
    }
    bool isValidBST(TreeNode* root) {
        stack<TreeNode *>stn;
        if(!root)return false;
        while(root)
        {
            stn.push(root);
            root = root->left;
        }
        TreeNode *p = stn.top();
        int val = p->val;
        while(!stn.empty())
        {
            TreeNode *p = stn.top();
            if(p->val < val)return false;
            val = p->val;
            stn.pop();
            if(p->right)
            {
                p = p->right;
                while(p)
                {
                    stn.push(p);
                    p = p->left;
                }
            }
        }
        return true;
    }
    vector<vector<int> > zigzagLevelOrder(TreeNode* root) {
        queue<TreeNode *>qtn;
        vector<vector<int> > vvi;
        if(!root)return vvi;
        qtn.push(root);
        int nCount = 1;
        int dir = 0;// 0 -->    1<---
        int i = 0;
        int j = 0;
        while(!qtn.empty())
        {
            vector<int>vi;
            while(j < nCount)
            {
                TreeNode * p = qtn.front();
                qtn.pop();
                vi.push_back(p->val);
                if(p->left){qtn.push(p->left); i++;}
                if(p->right){qtn.push(p->right); i++;}
                j++;
            }
            nCount = i;
            i = 0;
            j = 0;
            if(dir)reverse(vi.begin(),vi.end());
            dir = !dir;
            vvi.push_back(vi);
        }
        return vvi;
    }
    vector<vector<int> > levelOrder(TreeNode* root) {
        queue<TreeNode *>qtn;
        vector<vector<int> > vvi;
        if(!root)return vvi;
        qtn.push(root);
        int nCount = 1;
        int i = 0;// the total nums of node of next level
        int j = 0;// the number that has been passed of current level
        while(!qtn.empty())
        {
            vector<int>vi;
            while(j < nCount)
            {
                TreeNode * p = qtn.front();
                qtn.pop();
                vi.push_back(p->val);
                if(p->left){qtn.push(p->left); i++;}
                if(p->right){qtn.push(p->right); i++;}
                j++;
            }
            nCount = i;
            i = 0;
            j = 0;
            vvi.push_back(vi);
        }
        return vvi;
    }
    bool isSymmetric(TreeNode* root) {
        stack<TreeNode *>lstn;
        stack<TreeNode *>rstn;
        if(root==NULL)return true;
        if((!root->left && root->right)
         || (root->left && !root->right))return false;
        if(!root->left&&!root->right)return true;
        if(root->left->val!=root->right->val)return false;
        lstn.push(root->left);
        rstn.push(root->right);
        while(!lstn.empty() &&!rstn.empty())
        {
            TreeNode *pl = lstn.top();
            TreeNode *pr = rstn.top();
            lstn.pop();
            rstn.pop();
            if(pl->left && pr->right)
            {
                if(pl->left->val == pr->right->val){
                lstn.push(pl->left);
                rstn.push(pr->right);
                }
                else return false;
            }
            else if((pl->left && !pr->right)||(!pl->left && pr->right))return false;

            if(pl->right && pr->left)
            {
                if(pl->right->val == pr->left->val){
                lstn.push(pl->right);
                rstn.push(pr->left);
                }
                else return false;
            }
            else if((pl->right && !pr->left)||(!pl->right && pr->left))return false;

        }
        return true;
    }
    vector<int> preorderTraversal(TreeNode* root) {
        stack<TreeNode *> parents;
        vector<int> ans;
        if(root==NULL)return ans;
        parents.push(root);
        while(!parents.empty())
        {
            TreeNode*p = parents.top();
            ans.push_back(p->val);
            //printf("%d,",p->val);
            parents.pop();
            if(p->right)parents.push(p->right);
            if(p->left)parents.push(p->left);

        }
        return ans;
    }

    vector<int> preorderTraversal_re(TreeNode* root) {
        stack<TreeNode *> parents;
        vector<int> ans;
        if(root==NULL)return ans;
    #if 0
        void preorderTraversal__re_internal(vector<int> &ans,TreeNode *root)
        {
            ans.push_back(root->val);
            if(root->left)preorderTraversal__re_internal(ans,root->left);      
            if(root->right)preorderTraversal__re_internal(ans,root->right);
        }    
        preorderTraversal__re_internal(ans,root); 
    #else
        function<void(vector<int> &,TreeNode*)> preorderTraversal_re_internal_lambada = [&preorderTraversal_re_internal_lambada](vector<int> &ans,TreeNode *root)
        {
            ans.push_back(root->val);
            if(root->left)preorderTraversal_re_internal_lambada(ans,root->left);                    
            if(root->right)preorderTraversal_re_internal_lambada(ans,root->right);        
        }; 
        preorderTraversal_re_internal_lambada(ans,root);    
    #endif          
        return ans;
    }

    vector<int> inorderTraversal(TreeNode* root) {
        stack<TreeNode *> parents;
        vector<int> ans;
        if(root==NULL)return ans;
        while(root)
        {
            parents.push(root);
            root = root->left;
        }
        while(!parents.empty())
        {
            TreeNode *p = parents.top();
            ans.push_back(p->val);
            //printf("%d,",p->val);
            parents.pop();
            if(p->right)
            {
                p = p->right;
                while(p)
                {
                    parents.push(p);
                    p = p->left;
                }
            }
        }
        return ans;
    }

    vector<int> inorderTraversal_re(TreeNode* root) 
    {
        stack<TreeNode *> parents;
        vector<int> ans;
        if(root==NULL)return ans;
    #if 0    
        void inorderTraversal_re_internal(vector<int> &ans,TreeNode *root)
        {      
            if(root->left)inorderTraversal_re_internal(ans,root->left);  
            ans.push_back(root->val);    
            if(root->right)inorderTraversal_re_internal(ans,root->right);
        }
        inorderTraversal_re_internal(ans,root);  
    #else
        function<void(vector<int> &,TreeNode*)> inorderTraversal_re_internal_lambada = [&inorderTraversal_re_internal_lambada](vector<int> &ans,TreeNode *root)
        {
            if(root->left)inorderTraversal_re_internal_lambada(ans,root->left);   
            ans.push_back(root->val);         
            if(root->right)inorderTraversal_re_internal_lambada(ans,root->right);        
        }; 
        inorderTraversal_re_internal_lambada(ans,root);
    #endif          
        return ans;
    } 

    vector<int> postorderTraversal(TreeNode * root)
    {
        stack<TreeNode *> stn;
        vector<int> ans;
        if(root==NULL)return ans;
        TreeNode *pTemp = root;
        TreeNode *pLast;
        while(pTemp)
        {
            stn.push(pTemp);
            pTemp = pTemp->left;
        }
        while(!stn.empty())
        {
            pTemp = stn.top();
            while(pTemp->right &&pTemp->right!=pLast)
            {
                pTemp = pTemp->right;
                pLast = pTemp;
                stn.push(pTemp);
                while(pTemp->left)
                {
                    stn.push(pTemp->left);
                    pTemp = pTemp->left;
                    pLast = pTemp;
                }
            }
            pLast = pTemp;
            stn.pop();
            ans.push_back(pTemp->val);
        }
        return ans;  
    }

    vector<int> postorderTraversal_re(TreeNode * root)
    {
        stack<TreeNode *> parents;
        vector<int> ans;
        if(root==NULL)return ans;
    #if 0
        void postorderTraversal_re_internal(vector<int> &ans,TreeNode *root)
        {      
            if(root->left)postorderTraversal_re_internal(ans,root->left);            
            if(root->right)postorderTraversal_re_internal(ans,root->right);
            ans.push_back(root->val);
        } 
        //postorderTraversal_re_internal(ans,root);
    #else
        function<void(vector<int> &,TreeNode*)> postoderTraversal_re_internal_lambada = [&postoderTraversal_re_internal_lambada](vector<int> &ans,TreeNode *root)
        {
            if(root->left)postoderTraversal_re_internal_lambada(ans,root->left);            
            if(root->right)postoderTraversal_re_internal_lambada(ans,root->right);
            ans.push_back(root->val);
        }; 
        postoderTraversal_re_internal_lambada(ans,root);
    #endif    
        return ans;
    }

    bool isSameTree(TreeNode* p, TreeNode* q) {
        stack<TreeNode*>vtn_p;
        stack<TreeNode*>vtn_q;
        if((!p&&q)||(!q&&p)||((p&&q)&&(p->val!=q->val)))return false;
        if(!p&&!q)return true;
        vtn_p.push(p);
        vtn_q.push(q);

        while(!vtn_p.empty() && !vtn_q.empty())
        {
            TreeNode*pp = vtn_p.top();
            TreeNode*qq = vtn_q.top();
            //printf("pp:%x,qq:%x\r\n",pp,qq);
            if(pp && qq &&pp->val == qq->val)
            {
                vtn_p.pop();
                vtn_q.pop();
                if((!pp->left&&qq->left)||(!qq->left&&pp->left)){printf("1");return false;}
                if(pp->left &&qq->left){
                    vtn_p.push(pp->left);
                    vtn_q.push(qq->left);
                }
                if((!pp->right&&qq->right)||(!qq->right&&pp->right)){printf("2");return false;}
                if(pp->right&&qq->right)
                {
                    vtn_p.push(pp->right);
                    vtn_q.push(qq->right);
                }
            }
            else {printf("3");return false;}
        }
        return true;
    }
    int maxDepth(TreeNode* root) {
        int mDepth = 0;
        stack<TreeNode *>stn;
        if(!root)return mDepth;
        TreeNode*p = root;
        TreeNode*plast = NULL;
        while(p)
        {
            stn.push(p);
            p = p->left;
        }
        while(!stn.empty())
        {
            p = stn.top();
            while(p->right &&p->right!=plast)
            {
                p = p->right;
                plast = p;
                stn.push(p);
                while(p->left)
                {
                    stn.push(p->left);
                    p = p->left;
                    plast = p;
                }
            }
            plast = p;
            //printf("%d\r\n",stn.size());
            if(stn.size() > mDepth) mDepth = stn.size();
            stn.pop();
        }
        return mDepth;
    }
    int minDepth(TreeNode* root) {
        unsigned int minLeafDepth = 0xFFFFFFFF;
        stack<TreeNode *>stn;
        if(!root)return 0;
        TreeNode*p = root;
        TreeNode*plast = NULL;
        while(p)
        {
            stn.push(p);
            p = p->left;
        }
        while(!stn.empty())
        {
            p = stn.top();
            while(p->right &&p->right!=plast)
            {
                p = p->right;
                plast = p;
                stn.push(p);
                while(p->left)
                {
                    stn.push(p->left);
                    p = p->left;
                    plast = p;
                }
            }
            if(!p->left&&!p->right)
            {
                //printf("%d\r\n",stn.size());
                if(stn.size() < minLeafDepth) minLeafDepth = stn.size();
            }
            plast = p;
            stn.pop();
        }
        return minLeafDepth;
    }
    int sumOfLeftLeaves(TreeNode* root) {
        int numOfLeftLeaves = 0;
        stack<TreeNode *> stn;
        vector<int> ans;
        if(root==NULL)return numOfLeftLeaves;
        TreeNode*p = root;
        TreeNode*pp = NULL;
        TreeNode*plast = NULL;

        while(root)
        {
            stn.push(root);
            root = root->left;
        }
        while(!stn.empty())
        {
            p = stn.top();
            while(p->right &&p->right!=plast)
            {
                p = p->right;
                plast = p;
                stn.push(p);
                while(p->left)
                {
                    stn.push(p->left);
                    p = p->left;
                    plast = p;
                }
            }
            plast = p;
            stn.pop();
            if(stn.size()>=1){
                pp = stn.top();
                if(!p->left &&!p->right && p == pp->left)
                {
                    //printf("p:%d,pp:%d\r\n",p->val,pp->val);
                    numOfLeftLeaves+=p->val;
                }
            }
        }
        return numOfLeftLeaves;
    }
};

/*
     3
    / \
   9   20
      /  \
     15  7


    BST:
     3
      \
       9
      /  \
     7   20
        /
       15
*/





int main()
{
    //test();
    Solution s;
    int array[]={3,9,20,0,0,15,7};//0 present null
    //int array[]={1};
    //int array[]={2,1};
    //int array[]={1,0,2};
    //int array[]={1,2,2,3,4,4,3};
    //int array[]={1,2,2,0,3,0,3};
    //int array[]={1,2,3,0,5};
    //int array[]={10,5,15,3,7,13,18,1,0,6};
    //int array[]={3,1,4,0,0,2};
    //int array[]={5,3,6,2,4,0,7};
    //int array[]={1,0,2,2,3};
    //int array[]={5,4,8,11,0,13,4,7,2,0,0,5,1};
    
    int len = sizeof(array)/sizeof(array[0]);
    TreeNode * root = createBinaryTree(array,len);
    TreeNode * bstroot = createBinarySearchTree(array,len);
    vector<int>vi;
    //system_clock::time_point start,end;
    //microseconds duration;

    printf("preorder:");
    //start = system_clock::now();
    vi = s.preorderTraversal(root);
    //end = system_clock::now();
    //duration = duration_cast<microseconds>(end - start);
    /*
    不支持cout 只能用printf
    */
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
    for(auto item:vi)printf("%d,",item);
    printf("\tduration:%fs", double(duration.count()) * microseconds::period::num / microseconds::period::den);

    printf("\r\npreorder_re:");
    vi = s.preorderTraversal_re(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    printf("\r\ninorder:");
    vi = s.inorderTraversal(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    printf("\r\ninorder_re:");
    vi = s.inorderTraversal_re(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    printf("\r\npostorder:");
    vi = s.postorderTraversal(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    printf("\r\npostorder_re:");
    vi = s.postorderTraversal_re(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    int val = s.maxDepth(root);
    printf("\r\nmaxDepth:%d",val);

    val = s.minDepth(root);
    printf("\r\nminDepth:%d",val);

    val = s.sumOfLeftLeaves(root);
    printf("\r\nsumOfLeftLeaves:%d",val);

    val = s.isSymmetric(root);
    printf("\r\nisSymmetric:%s",val?"true":"false");

    vector<vector<int> > vvi;
    printf("\r\nlevelOrder:");
    vvi = s.levelOrder(root);
    for(vector<vector<int> >::iterator itr = vvi.begin();itr!=vvi.end();itr++)
    {
        //copy((*itr).begin(),(*itr).end(),ostream_iterator<int>(cout,","));
        for(auto item:(*itr))printf("%d,",item);
        printf("-->");
    }

    printf("\r\nzigzagLevelOrder:");
    vvi = s.zigzagLevelOrder(root);
    for(vector<vector<int> >::iterator itr = vvi.begin();itr!=vvi.end();itr++)
    {
        for(auto item:(*itr))printf("%d,",item);
        printf("-->");
    }

    printf("\r\nbst inorder:");
    vi = s.inorderTraversal(bstroot);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    val = s.isValidBST(bstroot);
    printf("\r\nisValidBST:%s",val?"true":"false");

    printf("\r\nbinaryTreePaths:");
    vector<string> vs =s.binaryTreePaths(root);
    //copy(vs.begin(),vs.end(),ostream_iterator<string>(cout,","));
	for(auto item:vs)printf("%s,",item.c_str());
    printf("\r\nlowestCommonAncestorBST:");
    TreeNode * parent = s.lowestCommonAncestorBST(bstroot,new TreeNode(9),new TreeNode(7));
    printf("%d",parent?parent->val:0);

    printf("\r\nlowestCommonAncestor:");
    parent = s.lowestCommonAncestor(root,new TreeNode(9),new TreeNode(15));
    printf("%d",parent?parent->val:0);

    val = s.hasPathSum(root,30);
    printf("\r\nhasPathSum:%s",val?"true":"false");

    val = s.sumNumbers(root);
    printf("\r\nsumNumbers:%d",val);

    val = s.countNodes(root);
    printf("\r\ncountNodes:%d",val);

    printf("\r\nfindMode:");
    vi = s.findMode(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
    val = s.kthSmallest(bstroot,3);
    printf("\r\nkthSmallest:%d",val);

    val = s.height(root);
    printf("\r\nheight:%d",val);

    val = s.isBalanced(root);
    printf("\r\nisBalanced:%s",val?"true":"false");

    printf("\r\npathSum:");
    vvi = s.pathSum(root,22);
    for(vector<vector<int> >::iterator itr = vvi.begin();itr!=vvi.end();itr++)
    {
        for(auto item:(*itr))printf("%d,",item);
        printf("-->");
    }

    printf("\r\nrightSideView:");
    vi = s.rightSideView(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
	for(auto item:vi)printf("%d,",item);
		
    printf("\r\nfindFrequentTreeSum:");
    vi = s.findFrequentTreeSum(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
    for(auto item:vi)printf("%d,",item);
///////////////////modify////////////////////////
#if 0
    s.recoverTree(root);
    printf("\r\nrecoverTreeToBST inorder_re:");
    vi = s.inorderTraversal_re(root);
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
    for(auto item:vi)printf("%d,",item);
#endif

    bstroot = s.deleteNode(bstroot,3);
    vi = s.inorderTraversal_re(bstroot);
    printf("\r\ndeleteNode inorder_re:");
    //copy(vi.begin(),vi.end(),ostream_iterator<int>(cout,","));
    for(auto item:vi)printf("%d,",item);
    return 0;
}
