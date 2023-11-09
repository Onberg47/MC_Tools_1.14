/*
Stephanos Balden
28/06/2023

Takes 1 input file and makes 1 new output file

Converts give-item commands (.mcfunction) to loot_table format (.json)
*/

#include <iostream>
#include <fstream>

using namespace std;

void replaceAllStr(std::string *strPtr, std::string seek, std::string replace) // searches a string(by address) seaching each position(minus the length of the seek phrase) replaceses all instances
{
    std::string sub, s;
    s = *strPtr;

    for (int i = 0; i < (s.length() - (seek.length() - 1)); i++)
    {
        sub = s.substr(i, seek.length());
        if (sub == seek)
        {
            s.replace(i, seek.length(), replace);
        }
    }
    *strPtr = s; // updates the original string passed by address
    return;      // left the return in to clarify that the function ends here
} // replaceAllStr()

int cntChar(std::string s, char c)
{
    int cnt = 0;

    for (int i = 0; i < s.length(); i++)
    {
        if (s[i] == c)
        {
            cnt++;
        }
    }
    return cnt;
} // cntChar()

int main()
{
    const string FILE_IN = "file.mcfunction";
    const string FILE_OUT = "file_out.json";

    string txt_in, token, nbt_data = "{}", name_data = "carrot_on_a_stick";

    ifstream inFile;  // input file
    ofstream outFile; // output file

    long pos = 0;     // stores the position in out_file
    int exec_cnt = 0; // a value to be used during debugging

    cout << "\nInitiating Build...\n\n";
    try
    {
        inFile.open(FILE_IN);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n! ERROR opening in-file at: " << FILE_IN << "\n!Build unsecessful!\n";
        return 0; // stops the program from proceeding, without putting everything in the try block. may be redundant as cerr terminates the program
    }
    try // try open outFile, should be flawless as even if a file exsists it will be overriden
    {
        outFile.open(FILE_OUT); // opening both file at once as the files will be read and written simultaneously
        outFile.clear();        // clear the file for overriding
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << "\n! ERROR opening out-file at: " << FILE_OUT << "\n!Build unsecessful!\n";
        return 0;
    }

    cout << "Files loaded sucessfully\n\n";
    outFile << "{\n\t\"type\": \"minecraft:entity\",\n\t\"pools\": [\n\t\t{" << endl;
    outFile << "\t\t\t\"rolls\": 1,\n\t\t\t\"entries\": [\n\n";

    while (getline(inFile, txt_in))
    {
        exec_cnt++;
        {
            // seperates the command into the name and nbt data, throwing away the rest
            int x = txt_in.find("minecraft:");
            token = txt_in.substr(x);
            int y = token.find("{");
            name_data = token.substr(0, y);

            string deliminator = "AttributeModifiers";
            x = deliminator.length() + 9; // not sure why the +9 is required to reach the correct position

            nbt_data = token.substr(y, (token.find(deliminator) - x));
            token = token.substr(token.find(deliminator));

            try
            {
                cout << exec_cnt << ": Modifying NBT data...\n";
                cout << exec_cnt << ": replacing \"display Name\"...\n";
                nbt_data.replace(nbt_data.find("\"Name\":"), 7, "Name:");
            }
            catch (const std::exception &e)
            {
                std::cout << e.what() << "\n! Could not find \"display Name\"\n";
            }

            // Single itteration replacements (there can only be 1 of this so a full search is unesessary)
            cout << exec_cnt << ": reformatting brackets...\n";
            nbt_data.replace(nbt_data.find("Name:\"{"), 7, "Name:\'{");
            nbt_data.replace(nbt_data.find("\",Lore:"), 7, "\',Lore:");

            // Full searches
            cout << exec_cnt << ": reformatting text...\n";
            replaceAllStr(&nbt_data, "\"\\\"", "\'\\\""); /*replaces: \"\  with '\"  */
            replaceAllStr(&nbt_data, "\\\"\"", "\\\"\'"); /*replaces: \"" with \"'  */
            replaceAllStr(&nbt_data, "\'\\\"\\\\", "\'\\\"\\"); /*replaces: '\"\\ with '\"\ for special characters  */

            cout << exec_cnt << ": Modifying Attrabutes...\n";
            cout << exec_cnt << ": removing quote...\n";
            for (int i = cntChar(token, '\"'); i > 0; i--)
            {
                x = token.find('\"');
                token.erase(x, 1);
            }

            cout << exec_cnt << ": recombining NBT data...\n";
            nbt_data += token; // recombines the NBT and Attrabute data
        }

        cout << exec_cnt << ": Writing data of entrie...\n";
        { // [blocked for ease of reading] This is the entries output after all data has been modified
            outFile << "{\n";
            outFile << "\t\"type\": \"minecraft:item\"," << endl;
            outFile << "\t\"weight\": 1," << endl;
            outFile << "\t\"functions\": [\n\t\t{" << endl;
            outFile << "\t\t\"function\": \"minecraft:set_nbt\"," << endl;
            outFile << "\t\t\"tag\": \"" << nbt_data << "\"\n\t\t}\n\t]," << endl;
            outFile << "\t\"name\":\"" << name_data << "\"\n},\n";
        }
    } // while file

    pos = outFile.tellp();
    outFile.seekp(pos - 1);
    outFile.put(' '); // replaces the last entrie's ',' with a whitespace

    outFile << "\n\t\t\t]\n\t\t}\n\t]\n}" << endl; // clsing all the brackets from the beginning of file

    cout << "\nBuild sucsses! Total commands reformatted: " << exec_cnt << " and exported at: " << FILE_OUT;
    cout << "\n\n### END OF PROGRAM ###\n\n";

    inFile.close();
    outFile.close();
    return 0;
}