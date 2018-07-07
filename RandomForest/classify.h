#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

void classifyInstances(TreeNode *root, vector<Attribute>& attributes, vector<vector<string>>& test, int treeindex);
char evaluate(vector<string>& instance, TreeNode *node, vector<Attribute>& attributes);

//function to classify all unknown instances and subsequently calculate the efficiency
void classifyInstances(TreeNode *root, vector<Attribute>& attributes, vector<vector<string>>& test, int treeindex)
{
	//classify each individual instance
	for (int i = 0; i < test.size(); i++)
	{
		test[i][15 + treeindex] = output_map_ctos[evaluate(test[i], root, attributes)];
	}
	//calculate efficiency
	float p = 0.0;
	for (int i = 0; i < test.size(); i++)
	{
		if (output_map[test[i][14]] == output_map[test[i][15+treeindex]])
		{
			p++;
		}
	}
	cout << "Accuracy of Tree " << treeindex+1 << " : " << (p / test.size())*100.0 << "%" << "\n";

}

//evaluates an instance as '+' or '-' and returns its evaluation
char evaluate(vector<string>& instance, TreeNode *node, vector<Attribute>& attributes)
{
	if (node->branch.empty())
	{
		return node->label;
	}
	else
	{
		char c;
		Attribute attr = attributes[attributes_map_ctoi[node->label]];
		for (int i = 0; i < node->branch.size(); i++)
		{
			if (attr.map[instance[attr.index]] == node->branch[i]->label)
			{
				c = evaluate(instance, node->branch[i]->child, attributes);
				return c;
			}
		}
	}

}
