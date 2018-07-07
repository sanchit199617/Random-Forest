// RandomForest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>	//std::distance
#include <ctime>

#include "attribute_maps.h"		//importing maps of all attributes
#include "threshold.h"			//for calculating the threshold of continous attributes (includes gain.h, attributes.structure.h, tree_structure.h)
#include "replace_unknowns.h"	//for replacing unknowns in the set
#include "perform_id3.h"		//to perform id3
#include "classify.h"			//to classify the test set


struct Tree
{
	TreeNode *root;
	vector<Attribute> continous_attributes;
	vector<map<string, int>> continous_attributes_map;
};

//function prototypes

void resize(vector<vector<string>>& data, ifstream& datafile);
void fillContAttrMaps(vector<Attribute>& cont_attr, vector<vector<string>>& test);
void classifyInstances(TreeNode *root, vector<Attribute>& attributes, vector<vector<string>>& test, int treeindex);
char evaluate(vector<string> instance, TreeNode *node, vector<Attribute> attributes);
TreeNode * formTree(vector<vector<string>>& data, vector<Attribute> attributes, Tree& Tree);
void fillContAttrMaps(vector<Attribute>& cont_attr, vector<vector<string>>& test, Tree& Tree);
void printTree(TreeNode *node);

int main()
{
	string line;
	ifstream datafile("datafiles/data.txt");
	ifstream testfile("testfiles/test.txt");
	fstream trainedfile;
	vector<vector<string>> data, test;
	vector<Tree> Trees;
	Trees.resize(10);

	clock_t begin = clock();
	//resize data
	resize(data, datafile);

	//extract data from data.txt and store to data vector
	datafile.open("datafiles/data.txt");
	if (datafile.is_open())
	{
		int i = 0;
		while (getline(datafile, line))
		{
			for (int j = 0, k = 0; j < line.length(); k++, j += 2)
			{
				while (line[j] != ',' && j < line.length())
				{
					data[i][k].push_back(line[j]);
					j++;
				}
			}
			i++;
		}
		datafile.close();
	}
	else
	{
		cout << "Unable to open data file!" << "\n";
	}

	//resize test
	resize(test, testfile);
	//extract test from test.txt and store to test vector
	testfile.open("testfiles/test.txt");
	if (testfile.is_open())
	{
		int i = 0;
		while (getline(testfile, line))
		{
			for (int j = 0, k = 0; j < line.length(); k++, j += 2)
			{
				while (line[j] != ',' && j < line.length())
				{
					test[i][k].push_back(line[j]);
					j++;
				}
			}
			i++;
		}
		testfile.close();
	}
	else
	{
		cout << "Unable to open test file!" << "\n";
	}

	//print size of dataset
	cout << "Dataset size : " << data.size() << "\n" << "\n";

	//define attributes
	Attribute age(0, 2), workclass(1, 8), fnlwgt(2, 2), education(3, 16), education_num(4, 2), marital_status(5, 7), occupation(6, 14);
	Attribute relationship(7, 6), race(8, 5), sex(9, 2), capital_gain(10, 2), capital_loss(11, 2), hours_per_week(12, 2), native_country(13, 41);

	//assigning maps to each nominal attribute
	workclass.map = workclass_map;
	education.map = education_map;
	marital_status.map = marital_map;
	occupation.map = occupation_map;
	relationship.map = relationship_map;
	race.map = race_map;
	sex.map = sex_map;
	native_country.map = country_map;

	vector<Attribute> attributes = { age , workclass, fnlwgt, education, education_num, marital_status, occupation, relationship, race, sex, capital_gain, capital_loss, hours_per_week, native_country };

	replaceUnknowns(data, attributes);		//replace unknowns in data file

	clock_t t1 = clock();
	//train n decision trees
	srand(time(NULL));
	for (int i = 0; i < Trees.size(); i++)
	{
		vector<Attribute> four_attributes;
		vector<Attribute> random_attr = attributes;
		random_shuffle(random_attr.begin(), random_attr.end());
		for (int i = 0; i < 4; i++)
		{
			four_attributes.push_back(random_attr[i]);
		}
		cout << "----------------- Tree " << i + 1 << " ------------------\n";
		for (int i = 0; i < 4; i++)
		{
			cout << "Attribute " << four_attributes[i].index << ", ";
		}
		cout << "\n\n";
		Trees[i].root = formTree(data, four_attributes, Trees[i]);
	}
	clock_t t2 = clock();
	double generaterf_secs = double(t2 - t1) / CLOCKS_PER_SEC;
	cout << string(43,'-') << "\n" << "Random Forest generated in " << generaterf_secs << " secs." << "\n";
	cout << string(43, '-') << "\n";
	
	//----------------------start testing----------------------

	
	//add more columns to test vector for accomodating the output predicted by our random forest
	for (int i = 0;i < test.size();i++)
	{
		test[i].resize(15+Trees.size());
	}

	cout << "\nTestset size : " << test.size() << "\n\n";

	replaceUnknowns(test, attributes);						//replace unknowns in test file

	cout << "Testing started...\n\n";
	//now classify
	clock_t t3 = clock();


	for (int i = 0; i < Trees.size(); i++)
	{
		//modify attributes according to test set
		//first fill map of each continous attribute that was present when forming this Tree, with values that this attribute takes in the test set
		vector<Attribute> cont_attr = { age, fnlwgt, education_num, capital_gain, capital_loss, hours_per_week };

		fillContAttrMaps(Trees[i].continous_attributes, test, Trees[i]); //fill maps of continous attributes of tree;
		
		for (int j = 0; j < cont_attr.size(); j++)
		{
			for (int k = 0; k < Trees[i].continous_attributes.size(); k++)
			{
				if (cont_attr[j].index == Trees[i].continous_attributes[k].index)
				{
					cont_attr[j].map = Trees[i].continous_attributes_map[k];
				}
			}
		}

		age.map = cont_attr[0].map;
		fnlwgt.map = cont_attr[1].map;
		education_num.map = cont_attr[2].map;
		capital_gain.map = cont_attr[3].map;
		capital_loss.map = cont_attr[4].map;
		hours_per_week.map = cont_attr[5].map;

		vector<Attribute> attributes_modified = { age , workclass, fnlwgt, education, education_num, marital_status, occupation, relationship, race, sex, capital_gain, capital_loss, hours_per_week, native_country };
		classifyInstances(Trees[i].root, attributes_modified, test, i);
	}

	for (int i = 0; i < test.size(); i++)
	{
		test[i].resize(test[i].size() + 1);
	}

	for (int i = 0; i < test.size(); i++)
	{
		int p = 0, n = 0;
		for (int j = 15; j < test[i].size() - 1; j++)
		{
			if (output_map[test[i][j]] == output_map[">50K"])
			{
				p++;
			}
			else if (output_map[test[i][j]] == output_map["<=50K"])
			{
				n++;
			}
		}
		if (p >= n)
		{
			test[i][test[i].size() - 1] = ">50K";
		}
		else
		{
			test[i][test[i].size() - 1] = "<=50K";
		}
	}
	float p = 0.0;
	for (int i = 0; i < test.size(); i++)
	{
		if (output_map[test[i][14]] == output_map[test[i][test[i].size() - 1]])
		{
			p++;
		}
	}
	cout << "\nAccuracy of Random Forest : " << (p / test.size())*100.0 << "%" << "\n";

	clock_t end = clock();
	double classify_secs = double(end - t3) / CLOCKS_PER_SEC;
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "\nTesting done in " << classify_secs << " secs." << "\n" << string(43, '-') << "\n";
	cout << "\nTotal running time : " << elapsed_secs << " secs." << "\n" << "\n";
	
    return 0;
}

//form decision tree on randomly selected instances and considering only 4 randomly selected attributes
TreeNode * formTree(vector<vector<string>>& data, vector<Attribute> attributes, Tree& Tree)
{
	TreeNode *root = new TreeNode;
	vector<vector<string>> randomdata;
	vector<Attribute> cont_attr;
	for (int i = 0; i < attributes.size(); i++)
	{
		if (attributes[i].index == 0 || attributes[i].index == 2 || attributes[i].index == 4 || attributes[i].index == 10 || attributes[i].index == 11 || attributes[i].index == 12)
		{
			cont_attr.push_back(attributes[i]);
		}
	}
	cont_attr_map.resize(cont_attr.size());
	for (int i = 0; i < data.size(); i++)
	{
		int n = (rand() / (float)RAND_MAX)*(data.size());
		randomdata.push_back(data[n]);
	}
	for (int i = 0; i < randomdata.size(); i++)
	{
		if (randomdata[i].size() == 0)
		{
			int n = (rand() / (float)RAND_MAX)*(data.size());
			randomdata[i] = data[n];
		}
	}
	clock_t t1 = clock();
	cout << "Finding Thresholds..." << "\n";
	for (int i = 0; i < cont_attr.size(); i++)
	{
		cont_attr[i].threshold = threshold(data, cont_attr[i]);
		cout << "Threshold of continous attribute " << cont_attr[i].index << " : " << cont_attr[i].threshold << "\n";
	}
	clock_t t2 = clock();
	double findthresholds_secs = double(t2 - t1) / CLOCKS_PER_SEC;
	cout << "\n" << "All thresholds founded in " << findthresholds_secs << " secs." << "\n" << "\n";
	Tree.continous_attributes = cont_attr;
	fillContAttrMaps(cont_attr, randomdata);
	Tree.continous_attributes_map = cont_attr_map;
	for (int i = 0; i < attributes.size(); i++)
	{
		for (int j = 0; j < cont_attr.size(); j++)
		{
			if (attributes[i].index == cont_attr[j].index)
			{
				attributes[i].map = cont_attr[j].map;
			}
		}
	}
	
	clock_t t3 = clock();
	cout << "Training started..." << "\n";

	root = id3(randomdata, attributes);
	for (int i = 0; i < cont_attr_map.size(); i++)
	{
		cont_attr_map[i].clear();
	}
	clock_t t4 = clock();
	double perform_id3_secs = double(t4 - t3) / CLOCKS_PER_SEC;
	cout << "\n" << "Training completed in " << perform_id3_secs << " secs." << "\n" << "\n";
	return root;
}

//resizing data and test file acording to inputted data.txt and test.txt
void resize(vector<vector<string>>& data, ifstream& datafile)
{
	string line;
	int size_data = 0;						//size of data (number of instances)
	if (datafile.is_open())
	{
		while (getline(datafile, line))
		{
			size_data++;
		}
		datafile.close();
	}
	else
	{
		cout << "Unable to open data file!" << "\n";
	}
	data.resize(size_data);
	for (int i = 0;i < data.size();i++)
	{
		data[i].resize(15);
	}
}

//filling maps of each continous attribute according to test/data set
void fillContAttrMaps(vector<Attribute>& cont_attr, vector<vector<string>>& test)
{
	for (int i = 0; i < cont_attr.size(); i++)
	{
		float t = cont_attr[i].threshold;
		for (int j = 0; j < test.size(); j++)
		{
			if (stoi(test[j][cont_attr[i].index]) <= t)
			{
				cont_attr_map[i][(test[j][cont_attr[i].index])] = 1;
			}
			else if (stoi(test[j][cont_attr[i].index]) > t)
			{
				cont_attr_map[i][(test[j][cont_attr[i].index])] = 2;
			}
		}
		cont_attr[i].map = cont_attr_map[i];
	}
}

void fillContAttrMaps(vector<Attribute>& cont_attr, vector<vector<string>>& test, Tree& Tree)
{
	for (int i = 0; i < cont_attr.size(); i++)
	{
		float t = cont_attr[i].threshold;
		for (int j = 0; j < test.size(); j++)
		{
			if (stoi(test[j][cont_attr[i].index]) <= t)
			{
				Tree.continous_attributes_map[i][(test[j][cont_attr[i].index])] = 1;
			}
			else if (stoi(test[j][cont_attr[i].index]) > t)
			{
				Tree.continous_attributes_map[i][(test[j][cont_attr[i].index])] = 2;
			}
		}
		cont_attr[i].map = Tree.continous_attributes_map[i];
	}
}

void printTree(TreeNode *node)
{
	if (!node->branch.empty())
	{
		cout << "N" << node->label << " ";
		for (int i = 0; i < (node->branch).size(); i++)
		{
			cout << node->label << "B" << (node->branch[i])->label;
			printTree((node->branch[i])->child);
		}
	}
	else
	{
		cout << node->label << " ";
	}
}
