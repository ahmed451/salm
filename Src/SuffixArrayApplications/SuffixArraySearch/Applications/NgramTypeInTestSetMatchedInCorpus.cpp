#include "stdio.h"
#include "stdlib.h"

#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream, std::stringbuf
#include <vector>

#include "_String.h"
#include "_SuffixArraySearchApplicationBase.h"

#include <time.h>
#include <stdio.h>
#include <map>
#include <set>
#include <tuple> 

#include <algorithm> // std::sort

using namespace std;

typedef pair<C_String, double> pairElt;

// Convert Map to Vec and Sort by value AAA
std::vector<pairElt> SortMap2Vec(map<C_String, double> myMap){
	// 
	std::vector<pairElt >::const_iterator itvec;
	map<C_String, double>::iterator itmap;
	vector<pair<C_String, double> > order(myMap.size());

	size_t n = 0;
	for (itmap = myMap.begin(); itmap != myMap.end(); ++itmap, ++n)
    	order[n] = make_pair((itmap->first).toString(),itmap->second);


	//Now you can sort this array using a custom sorter:

	struct ordering {
    	bool operator ()(pairElt const& a, pairElt const& b) {
        	return (a.second) > (b.second); // descending order 
    	}
	};

	sort(order.begin(), order.end(), ordering());

	return order;
}


vector<C_String> convertTextToStringVector(const char * sentText)
{  

    vector<C_String> sentAsStringVect;

    char tmpToken[MAX_TOKEN_LEN];
    memset(tmpToken,0,MAX_TOKEN_LEN);

    int pos = 0;

    int inputLen = strlen(sentText);

    for(int posInInput = 0; posInInput<inputLen; posInInput++){
        char thisChar = sentText[posInInput];

        if((thisChar==' ')||(thisChar=='\t')){  //delimiters
            if(strlen(tmpToken)>0){
                tmpToken[pos] = '\0';               
                sentAsStringVect.push_back(C_String(tmpToken));
                pos=0;
                tmpToken[pos] = '\0';
            }
        }
        else{
            tmpToken[pos] = thisChar;
            pos++;
            if(pos>=MAX_TOKEN_LEN){ //we can handle it
                fprintf(stderr,"Can't read tokens that exceed length limit %d. Quit.\n", MAX_TOKEN_LEN);
                exit(0);
            }
        }
    }

    tmpToken[pos] = '\0';
    if(strlen(tmpToken)>0){     
        sentAsStringVect.push_back(C_String(tmpToken));
    }

    return sentAsStringVect;
}

/*
*
*
*/

int word_count(string is)  // can pass an open std::ifstream() to this if required
{
	int word_count( 0 );
    stringstream ss( is );
    string word;
    while( ss >> word ) ++word_count;
    return word_count;

}

/**
*
* Load Corpus file and build <doc ids, doc names> map
*
**/
map<double, C_String> LoadDocLines(const char * fileName){
	map<double, C_String> listDocLine; 
	ifstream textStream;
	string aLine;
	double nLine = 0;
	char * thisToken;
	char delimit[] =" \t\r\n";

	textStream.open(fileName);
	if(textStream==NULL){
		fprintf(stderr,"Corpus file %s does not exist. Exit!\n",fileName);
		exit(-1);
	}

	getline(textStream, aLine);
	while(!textStream.eof()){

		if(aLine.length()>0){

			thisToken = strtok((char*) aLine.c_str(), delimit );
			//cout<<" Line ["<< nLine<<"]: " <<thisToken << endl;
			nLine++;
			listDocLine.insert(make_pair(nLine,thisToken));


		}
		
		getline(textStream, aLine);
	}
	//textStream.close();
	return listDocLine;
}
/**
* \ingroup search
*
* Given the training corpus indexed by its suffix array,
* output all the n-grams in a testing data that can be found in the training corpus
*
* Revision $Rev: 3794 $
* Last Modified $LastChangedDate: 2007-06-29 02:17:32 -0400 (Fri, 29 Jun 2007) $
**/
int main(int argc, char* argv[]){
	//-----------------------------------------------------------------------------
	//check parameter


	if(argc<2){		
		fprintf(stderr,"\nOutput the matched n-gram types a testing data set given an indexed corpus\n");
		fprintf(stderr,"\nUsage:\n");
		fprintf(stderr,"\n%s corpusFileNameStem verbosity < testing data\n\n",argv[0]);
		
		exit(0);
	}


	//-----------------------------------------------------------------------------	
		
	C_SuffixArraySearchApplicationBase SA;
	
	map<C_String, double> matchedNgrams;
	map<C_String, double>::iterator iterMatchedNgrams;
	map<double, C_String> line2Doc;
	map<double, C_String>::iterator iterline2Doc;


	map<C_String, double> FoundFreq;
	map<C_String, double>::iterator iterFoundFreq;

	int maxSentLen = 4086;

	int verbosity = 0;

	char fileName[1000];
	char tmpString[10000];

	C_String l_ngram;
	double l_frequency;

	strcpy(fileName, argv[1]);

	if(argc>2)
		if(atoi(argv[2])>0)
			verbosity = 1;
		
	
	fprintf(stderr,"Loading data...\n");
	SA.loadData_forSearch(fileName, false, false); //true

	line2Doc = LoadDocLines(fileName);

	if(verbosity)
		cout <<"Input sentences:\n";

	long ltime1, ltime2;

	time( &ltime1 );

	int totalSentences = 0;
	int matchedSentences = 0;

	while(!cin.eof()){
		cin.getline(tmpString,10000,'\n');

		if(strlen(tmpString)>0) {
			vector<C_String> sentAsStringVector = convertTextToStringVector(tmpString);
			
			int sentLen;
			int CurrStart = 0;

			S_sentSearchTableElement * freqTable = SA.constructNgramSearchTable4SentWithLCP(tmpString, sentLen);
		  	
			if(sentLen!=sentAsStringVector.size()){
				cerr<<"Something wrong, can not proceed.!\n";
				exit(-1);
			}
			if(verbosity)
				cout << "\nProcessing :" << tmpString << endl;
			//go over the frequency table
			for(int startPos = 0; startPos<=sentLen; startPos++){
				C_String ngram;
				bool stillMatching = true;
				int n=1;
				while(stillMatching & (n<=(sentLen-startPos)) ){
				
					ngram.appending(sentAsStringVector[startPos+n-1]);

					int posInFreqTable = (n-1)*sentLen+startPos;
					if(freqTable[posInFreqTable].found){
						double frequency = freqTable[posInFreqTable].endingPosInSA - freqTable[posInFreqTable].startPosInSA + 1;

						iterMatchedNgrams = matchedNgrams.find(ngram);
						if(iterMatchedNgrams!=matchedNgrams.end()){	//exist already
							iterMatchedNgrams->second=frequency;	//frequency is not meaningful in this case, just use it because map need some values to be mapped to
						}
						else{
							matchedNgrams.insert(make_pair(ngram, frequency));
							//cout<<"M: "<<ngram.toString()<<","<< frequency<< ","<< n <<","<< (sentLen-startPos)<< ", "<<stillMatching <<endl;
							//if(startPos == 0) {
								l_ngram = ngram;
								l_frequency = frequency;
							}
							//}
					}
					else{
						stillMatching = false;
					}
					

					ngram.appending(C_String(" "));

					n++;
				}

					//
					// Get sentIDs and Document names AAA Tue Apr 10 08:30:34 2018
					// 
					std::vector<S_phraseLocationElement>::const_iterator i;
				
					vector<S_phraseLocationElement> locations;
					locations = SA.findPhrasesInASentence(l_ngram.toString());
					int start =(int)locations[locations.size()-1].posStartInSrcSent;
					int end   = (int)locations[locations.size()-1].posEndInSrcSent;
					int selID = locations.size()-1;
					CurrStart = startPos+(int)(locations[locations.size()-1].posEndInSrcSent);

					iterline2Doc = line2Doc.find(locations[selID].sentIdInCorpus);
					if(verbosity) {
						cout <<l_ngram.toString()<<" ["<< word_count(l_ngram.toString()) << ", "<< l_frequency << ", n:"<< n <<", Len:"<< sentLen<< ", Strt:"<< startPos<< ", Diff:"<<(sentLen-startPos)<< ", Flag:"<<CurrStart<< " "; // << endl;
						cout << (int)locations[selID].posInSentInCorpus << ", ";
						cout << (iterline2Doc->second).toString() << "]" << endl;
					}
					
					
					
					startPos = startPos+((int)locations[selID].posEndInSrcSent-(int)locations[selID].posStartInSrcSent);
		
					if(FoundFreq.end() == FoundFreq.find((iterline2Doc->second).toString()))
								{
									FoundFreq.insert(make_pair((iterline2Doc->second).toString(), word_count(l_ngram.toString())));
								}
					else
						FoundFreq.find((iterline2Doc->second))->second += word_count(l_ngram.toString());
				//}
				//CurrStart = 
				
			}

			
		} // if(strlen(tmpString)>0)

		tmpString[0]=0;

		
		
	} // While
 
 	//
 	// Output sorted list AAA
 	//

	std::vector<std::pair<C_String, double> > items;
	items = SortMap2Vec(FoundFreq);
	std::vector<std::pair<C_String,double> >::const_iterator it;
	for(it=items.begin(); it!=items.end(); ++it)
		cout << "Document[" <<(it->first).toString()  <<"] = " <<   it->second << endl;



	return 1;
	cout<<"SSS: "<<l_ngram.toString()<<","<< l_frequency<<endl;

	//now output all the n-grams
	iterMatchedNgrams = matchedNgrams.begin();
	while(iterMatchedNgrams != matchedNgrams.end()){
		cout<<(iterMatchedNgrams->first).toString()<<","<< iterMatchedNgrams->second<<endl;

		iterMatchedNgrams++;
	}


	time( &ltime2 );	
	cerr<<"Time spent:"<<ltime2-ltime2<<" seconds\n";

	return 1;
}
