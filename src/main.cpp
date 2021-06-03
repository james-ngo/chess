/*
ZJ Wood, Dunn, Eckhardt CPE 471 Lab base code
*/

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
Shape rook, knight, bishop, queen, king, pawn;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

bool correct_player(int current_player, char piece_color) {
	return (current_player == 1 && piece_color == 'W') || (current_player == -1 && piece_color == 'B');
}

char current_player_color(int current_player) {
	if (current_player == 1) {
		return 'W';
	}
	return 'B';
}

bool valid_coord(int i, int j) {
	return i >= 0 && i <= 7 && j >= 0 && j <= 7;
}


string getString(char x)
{
	string s(1, x);

	return s;
}

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e;
	camera()
	{
		w = a = s = d = q = e = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		float yspeed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		if (q == 1)
		{
			yspeed = 10 * ftime;
		}
		else if (e == 1)
		{
			yspeed = -10 * ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, yspeed, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

//game state variables

string board[8][8] = { {"WR", "WN", "WB", "WQ", "WK", "WB", "WN", "WR"},
			 		   {"WP", "WP", "WP", "WP", "WP", "WP", "WP", "WP"},
					   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
					   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
					   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
					   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
					   {"BP", "BP", "BP", "BP", "BP", "BP", "BP", "BP"},
					   {"BR", "BN", "BB", "BQ", "BK", "BB", "BN", "BR"} };
string calcBoard[8][8] = { {"WR", "WN", "WB", "WQ", "WK", "WB", "WN", "WR"},
					       {"WP", "WP", "WP", "WP", "WP", "WP", "WP", "WP"},
						   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
						   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
						   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
						   {"OO", "OO", "OO", "OO", "OO", "OO", "OO", "OO"},
						   {"BP", "BP", "BP", "BP", "BP", "BP", "BP", "BP"},
						   {"BR", "BN", "BB", "BQ", "BK", "BB", "BN", "BR"} };

int selectedSquare[2] = { 0, 0 };
int pieceToMove[2] = { -1, -1 };
int destination[2] = { -1, -1 };
int pieceSelected = false;
int current_player = 1;
bool en_passant_available = false;
int en_passant_pawn[2] = { -1, -1 };
vector<vector<int>> valid_moves;
bool wqr = false; //white queenside rook moved?
bool wkr = false; //white kingside rook moved?
bool wk = false; //white king moved?
bool bqr = false; //black queenside rook moved?
bool bkr = false; //black kingside rook moved?
bool bk = false; //black1 king moved?
int white_king_pos[2] = { 0, 4 };
int black_king_pos[2] = { 7, 4 };


bool en_passant(int current_player, int i, int j, int direction) {
	if (!((current_player == 1 && j == 4) || (current_player == -1 && j == 3))) {
		return false;
	}
	return (en_passant_available && board[j][i + direction] == getString(current_player_color(-1 * current_player)) + 'P');
}

vector<vector<int>> valid_pawn_moves(int current_player, int i, int j) {
	vector<vector<int>> ret;
	// checking normal moves
	if (j == 1 || j == 6) {
		vector<int> vec{ i, j + current_player};
		int moves = 0;
		while (calcBoard[vec[1]][vec[0]] == "OO" && moves < 2) {
			ret.push_back(vec);
			vec = { vec[0], vec[1] + current_player };
			moves += 1;
		}
	}
	else {
		if (calcBoard[j + current_player][i] == "OO") {
			vector<int> vec = { i, j + current_player};
			ret.push_back(vec);
		}
	}
	// checking captures
	vector<int> vec;
	if (i < 7 && (calcBoard[j + current_player][i + 1][0] == current_player_color(-1 * current_player) ||
				  en_passant(current_player, i, j, 1))) {
		vec = { i + 1, j + current_player };
		ret.push_back(vec);
	}
	if (i > 0 && (calcBoard[j + current_player][i - 1][0] == current_player_color(-1 * current_player) ||
				  en_passant(current_player, i, j, -1))) {
		vec = { i - 1, j + current_player };
		ret.push_back(vec);
	}
	// en passant LMAO
	return ret;
}

vector<vector<int>> valid_knight_moves(int current_player, int i, int j) {
	int x[8] = { -1, 1, -2, 2, -1, 1, -2, 2 };
	int y[8] = { -2, 2, -1, 1, 2, -2, 1, -1 };
	vector<int> vec;
	vector<vector<int>> ret;
	for (int k = 0; k < 8; k++) {
		if (valid_coord(i + x[k], j + y[k]) && (calcBoard[j + y[k]][i + x[k]] == "OO" || calcBoard[j + y[k]][i + x[k]][0] == current_player_color(-1 * current_player))) {
			vec = { i + x[k], j + y[k] };
			ret.push_back(vec);
		}
	}
	return ret;
}

vector<vector<int>> valid_bishop_moves(int current_player, int i, int j) {
	vector<int> vec;
	vector<vector<int>> ret;
	int temp_i = i;
	int temp_j = j;
	while (valid_coord(temp_i + 1, temp_j + 1)) {
		if (calcBoard[temp_j + 1][temp_i + 1] == "OO") {
			vec = { temp_i + 1, temp_j + 1 };
			ret.push_back(vec);
			temp_i += 1;
			temp_j += 1;
		}
		else if (calcBoard[temp_j + 1][temp_i + 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i + 1, temp_j + 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_i = i;
	temp_j = j;
	while (valid_coord(temp_i + 1, temp_j - 1)) {
		if (calcBoard[temp_j - 1][temp_i + 1] == "OO") {
			vec = { temp_i + 1, temp_j - 1 };
			ret.push_back(vec);
			temp_i += 1;
			temp_j -= 1;
		}
		else if (calcBoard[temp_j - 1][temp_i + 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i + 1, temp_j - 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_i = i;
	temp_j = j;
	while (valid_coord(temp_i - 1, temp_j + 1)) {
		if (calcBoard[temp_j + 1][temp_i - 1] == "OO") {
			vec = { temp_i - 1, temp_j + 1 };
			ret.push_back(vec);
			temp_i -= 1;
			temp_j += 1;
		}
		else if (calcBoard[temp_j + 1][temp_i - 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i - 1, temp_j + 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_i = i;
	temp_j = j;
	while (valid_coord(temp_i - 1, temp_j - 1)) {
		if (calcBoard[temp_j - 1][temp_i - 1] == "OO") {
			vec = { temp_i - 1, temp_j - 1 };
			ret.push_back(vec);
			temp_i -= 1;
			temp_j -= 1;
		}
		else if (calcBoard[temp_j - 1][temp_i - 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i - 1, temp_j - 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	return ret;
}

vector<vector<int>> valid_rook_moves(int current_player, int i, int j) {
	vector<int> vec;
	vector<vector<int>> ret;
	int temp_i = i;
	int temp_j = j;
	while (valid_coord(temp_i + 1, temp_j)) {
		if (calcBoard[temp_j][temp_i + 1] == "OO") {
			vec = { temp_i + 1, temp_j};
			ret.push_back(vec);
			temp_i += 1;
		}
		else if (calcBoard[temp_j][temp_i + 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i + 1, temp_j };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_i = i;
	while (valid_coord(temp_i - 1, temp_j)) {
		if (calcBoard[temp_j][temp_i - 1] == "OO") {
			vec = { temp_i - 1, temp_j };
			ret.push_back(vec);
			temp_i -= 1;
		}
		else if (calcBoard[temp_j][temp_i - 1][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i - 1, temp_j };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_i = i;
	while (valid_coord(temp_i, temp_j + 1)) {
		if (calcBoard[temp_j + 1][temp_i] == "OO") {
			vec = { temp_i, temp_j + 1 };
			ret.push_back(vec);
			temp_j += 1;
		}
		else if (calcBoard[temp_j + 1][temp_i][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i, temp_j + 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	temp_j = j;
	while (valid_coord(temp_i, temp_j - 1)) {
		if (calcBoard[temp_j - 1][temp_i] == "OO") {
			vec = { temp_i, temp_j - 1 };
			ret.push_back(vec);
			temp_j -= 1;
		}
		else if (calcBoard[temp_j - 1][temp_i][0] == current_player_color(-1 * current_player)) {
			vec = { temp_i, temp_j - 1 };
			ret.push_back(vec);
			break;
		}
		else {
			break;
		}
	}
	return ret;
}

vector<vector<int>> valid_queen_moves(int current_player, int i, int j) {
	vector<vector<int>> rmoves = valid_rook_moves(current_player, i, j);
	vector<vector<int>> bmoves = valid_bishop_moves(current_player, i, j);
	vector<vector<int>> ret(rmoves);
	ret.insert(ret.end(), bmoves.begin(), bmoves.end());
	return ret;
}

vector<vector<int>> valid_king_moves(int current_player, int i, int j) {
	int x[9] = { 0, 0, -1, -1, -1, 1, 1, 1 };
	int y[9] = { 1, -1, 0, -1, 1, 0, -1, 1 };
	vector<int> vec;
	vector<vector<int>> ret;
	for (int k = 0; k < 8; k++) {
		if (valid_coord(i + x[k], j + y[k]) && (calcBoard[j + y[k]][i + x[k]] == "OO" || calcBoard[j + y[k]][i + x[k]][0] == current_player_color(-1 * current_player))) {
			vec = { i + x[k], j + y[k] };
			ret.push_back(vec);
		}
	}
	//check for castling
	if (current_player == 1 && !wk && !wkr && calcBoard[j][i + 1] == "OO" && calcBoard[j][i + 2] == "OO") {
		vec = { i + 2, j };
		ret.push_back(vec);
	}
	if (current_player == 1 && !wk && !wqr && calcBoard[j][i - 1] == "OO" && calcBoard[j][i - 2] == "OO" && calcBoard[j][i - 3] == "OO") {
		vec = { i - 2, j };
		ret.push_back(vec);
	}
	if (current_player == -1 && !bk && !bkr && calcBoard[j][i + 1] == "OO" && calcBoard[j][i + 2] == "OO") {
		vec = { i + 2, j };
		ret.push_back(vec);
	}
	if (current_player == -1 && !bk && !bqr && calcBoard[j][i - 1] == "OO" && calcBoard[j][i - 2] == "OO" && calcBoard[j][i - 3] == "OO") {
		vec = { i - 2, j };
		ret.push_back(vec);
	}
	return ret;
}

vector<int> get_king_pos(int current_player) {
	vector<int> vec;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (calcBoard[i][j] == getString(current_player_color(current_player)) + getString('K')) {
				vec = { i, j };
				return vec;
			}
		}
	}
}

bool king_in_check(int current_player) {
	vector<vector<int>> moves;
	vector<int> current_king_pos = get_king_pos(current_player);
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (calcBoard[i][j][0] == current_player_color(-1 * current_player)) {
				if (calcBoard[i][j][1] == 'P') {
					moves = valid_pawn_moves(-1 * current_player, j, i);
				}
				if (calcBoard[i][j][1] == 'R') {
					moves = valid_rook_moves(-1 * current_player, j, i);
				}
				if (calcBoard[i][j][1] == 'N') {
					moves = valid_knight_moves(-1 * current_player, j, i);
				}
				if (calcBoard[i][j][1] == 'B') {
					moves = valid_bishop_moves(-1 * current_player, j, i);
				}
				if (calcBoard[i][j][1] == 'Q') {
					moves = valid_queen_moves(-1 * current_player, j, i);
				}
				for (int k = 0; k < moves.size(); k++) {
					if (moves[k][1] == current_king_pos[0] && moves[k][0] == current_king_pos[1]) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, prog2, prog3, prog4, prog5;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexColorIDBox, IndexBufferIDBox;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
			if (!pieceSelected && correct_player(current_player, board[selectedSquare[1]][selectedSquare[0]][0])) {
				memcpy(pieceToMove, selectedSquare, sizeof(int) * 2);
				pieceSelected = !pieceSelected;
			}
			else if (pieceSelected) {
				memcpy(destination, selectedSquare, sizeof(int) * 2);
				pieceSelected = !pieceSelected;
			}
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			if (selectedSquare[0] + current_player <= 7 && selectedSquare[0] + current_player >= 0) {
				selectedSquare[0] += current_player;
			}
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			if (selectedSquare[0] - current_player <= 7 && selectedSquare[0] - current_player >= 0) {
				selectedSquare[0] -= current_player;
			}
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			if (selectedSquare[1] - current_player <= 7 && selectedSquare[1] - current_player >= 0) {
				selectedSquare[1] -= current_player;
			}
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			if (selectedSquare[1] + current_player <= 7 && selectedSquare[1] + current_player >= 0) {
				selectedSquare[1] += current_player;
			}
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		//double posX, posY;
		//float newPt[2];
		//if (action == GLFW_PRESS)
		//{
		//	glfwGetCursorPos(window, &posX, &posY);
		//	std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

		//	//change this to be the points converted to WORLD
		//	//THIS IS BROKEN< YOU GET TO FIX IT - yay!
		//	newPt[0] = 0;
		//	newPt[1] = 0;

		//	std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
		//	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
		//	//update the vertex array with the updated points
		//	glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
		//	glBindBuffer(GL_ARRAY_BUFFER, 0);
		//}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../resources" ;
		// Initialize mesh.
	
		rook.loadMesh(resourceDirectory + "/Rook.obj");
		rook.resize();
		rook.init();

		knight.loadMesh(resourceDirectory + "/Knight.obj");
		knight.resize();
		knight.init();

		bishop.loadMesh(resourceDirectory + "/Bishop.obj");
		bishop.resize();
		bishop.init();

		queen.loadMesh(resourceDirectory + "/Queen.obj");
		queen.resize();
		queen.init();

		king.loadMesh(resourceDirectory + "/King.obj");
		king.resize();
		king.init();

		pawn.loadMesh(resourceDirectory + "/Pawn.obj");
		pawn.resize();
		pawn.init();

		// square

		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		glGenBuffers(1, &VertexBufferID);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		GLfloat cube_vertices[] = {
			// front
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			// back
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0,
			//tube 8 - 11
			-1.0, -1.0,  1.0,
			1.0, -1.0,  1.0,
			1.0,  1.0,  1.0,
			-1.0,  1.0,  1.0,
			//12 - 15
			-1.0, -1.0, -1.0,
			1.0, -1.0, -1.0,
			1.0,  1.0, -1.0,
			-1.0,  1.0, -1.0
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//color
		GLfloat cube_colors[] = {
			// front colors
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			1.0, 0.0, 0.5,
			// back colors
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			0.5, 0.5, 0.0,
			// tube colors
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
			0.0, 1.0, 1.0,
		};
		glGenBuffers(1, &VertexColorIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VertexColorIDBox);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort cube_elements[] = {

			// front
			0, 1, 2,
			2, 3, 0,
			// back
			7, 6, 5,
			5, 4, 7,
			//tube 8-11, 12-15
			8,12,13,
			8,13,9,
			9,13,14,
			9,14,10,
			10,14,15,
			10,15,11,
			11,15,12,
			11,12,8

		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

		glBindVertexArray(0);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.52f, 0.8f, 0.92f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Enable blending/transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		// white
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");

		
		// black
		prog2 = std::make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/shader_vertex2.glsl", resourceDirectory + "/shader_fragment2.glsl");
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addUniform("campos");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertNor");

		// red highlighting box
		prog3 = std::make_shared<Program>();
		prog3->setVerbose(true);
		prog3->setShaderNames(resourceDirectory + "/shader_vertex3.glsl", resourceDirectory + "/shader_fragment3.glsl");
		prog3->init();
		prog3->addUniform("P");
		prog3->addUniform("V");
		prog3->addUniform("M");
		prog3->addUniform("campos");
		prog3->addAttribute("vertPos");
		prog3->addAttribute("vertNor");

		// green available moves
		prog4 = std::make_shared<Program>();
		prog4->setVerbose(true);
		prog4->setShaderNames(resourceDirectory + "/shader_vertex4.glsl", resourceDirectory + "/shader_fragment4.glsl");
		prog4->init();
		prog4->addUniform("P");
		prog4->addUniform("V");
		prog4->addUniform("M");
		prog4->addUniform("campos");
		prog4->addAttribute("vertPos");
		prog4->addAttribute("vertNor");
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P, ball_movement; //View, Model and Perspective matrix

		P = glm::perspective((float)(3.141592 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		float trans = 0;// sin(t) * 2;
		glm::mat4 RotateZ;
		float angle = -3.1415926/2.0;


		glm::mat4 liftCam = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));
		glm::mat4 rotateCam = glm::rotate(glm::mat4(1.0f), 0.75f, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 transCam = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 11.9f));
		glm::mat4 spinCam = glm::rotate(glm::mat4(1.0f), 3.14159f, glm::vec3(0.0f, 1.0f, 0.0f));

		// Draw the box using GLSL.
		V = mycam.process(frametime);
		V = rotateCam * liftCam * V;

		// checker matrices

		glm::mat4 S_checker = glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.05f, 0.35f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

		//update board if user moves a piece
		if (pieceToMove[0] != -1 && destination[0] != -1 && correct_player(current_player, board[pieceToMove[1]][pieceToMove[0]][0])) {
			bool moveIsValid = false;
			for (int i = 0; i < valid_moves.size(); i++) {
				if (valid_moves[i][0] == destination[0] && valid_moves[i][1] == destination[1]) {
					moveIsValid = true;
				}
			}
			if (!std::equal(std::begin(pieceToMove), std::end(pieceToMove), std::begin(destination)) && moveIsValid) {
				//remove pawn in the event of en passant
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'P' && pieceToMove[0] != destination[0] && en_passant_available && destination[0] == en_passant_pawn[0]) {
					board[en_passant_pawn[1]][en_passant_pawn[0]] = "OO";
				}
				//check for double move pawn first move
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'P' && abs(pieceToMove[1] - destination[1]) == 2) {
					en_passant_available = true;
					en_passant_pawn[0] = destination[0];
					en_passant_pawn[1] = destination[1];
				}
				else {
					en_passant_available = false;
				}
				//check if rook or king has moved
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'R' && pieceToMove[1] == 0 && pieceToMove[0] == 0) {
					wkr = true;
				}
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'R' && pieceToMove[1] == 0 && pieceToMove[0] == 7) {
					wqr = true;
				}
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'R' && pieceToMove[1] == 7 && pieceToMove[0] == 0) {
					bkr = true;
				}
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'R' && pieceToMove[1] == 7 && pieceToMove[0] == 7) {
					bqr = true;
				}
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'K' && current_player == 1) {
					wk = true;
				}
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'K' && current_player == -1) {
					bk = true;
				}
				//castle
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'K' && abs(pieceToMove[0] - destination[0]) == 2) {
					if (destination[0] > 4) {
						board[pieceToMove[1]][7] = "OO";
						board[pieceToMove[1]][destination[0] - 1] = getString(current_player_color(current_player)) + getString('R');
					}
					else {
						board[pieceToMove[1]][0] = "OO";
						board[pieceToMove[1]][destination[0] + 1] = getString(current_player_color(current_player)) + getString('R');
					}
				}
				//check modifying king position
				if (board[pieceToMove[1]][pieceToMove[0]][1] == 'K') {
					if (current_player == 1) {
						white_king_pos[0] = destination[0];
						white_king_pos[1] = destination[1];
					}
				}
				board[destination[1]][destination[0]] = board[pieceToMove[1]][pieceToMove[0]];
				board[pieceToMove[1]][pieceToMove[0]] = "OO";
				for (int x = 0; x < 8; x++) {
					for (int y = 0; y < 8; y++) {
						calcBoard[x][y] = board[x][y];
					}
				}
				current_player *= -1;
				if (current_player == -1) {
					mycam.pos = vec3(0, 0, 11.9);
					mycam.rot = vec3(0, 3.14159, 0);
				}
				else {
					mycam.pos = vec3(0, 0, 0);
					mycam.rot = vec3(0, 0, 0);
				}
			}
			pieceToMove[0] = -1;
			pieceToMove[1] = -1;
			destination[0] = -1;
			destination[1] = -1;
			valid_moves.clear();
		}

		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				if ((i + j) % 2 == 0) {
					prog2->bind();
				}
				else {
					prog->bind();
				}
				glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(i * 0.7f - 2.5f, 0.0f, -0.7f * j - 0.5f));
				M = T * TransZ * S_checker;


				//send the matrices to the shaders
				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
				glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

				glBindVertexArray(VertexArrayID);
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
			
				if ((i + j) % 2 == 0) {
					prog2->unbind();
				}
				else {
					prog->unbind();
				}


				if (board[j][i][0] == 'B') {
					prog2->bind();
				}
				else if (board[j][i][0] == 'W') {
					prog->bind();
				}

				// draw piece

				glm::mat4 S_piece = glm::scale(glm::mat4(1.0f), glm::vec3(0.4f, 0.4f, 0.4f));
				T = glm::translate(glm::mat4(1.0f), glm::vec3(i * 0.7f - 2.5f, 0.5f, -0.7f * j - 0.5f));

				M = T * TransZ * S_piece;

				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
				glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
				glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);

				if (board[j][i][1] == 'R') {
					S_piece = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
					T = glm::translate(glm::mat4(1.0f), glm::vec3(i * 0.7f - 2.5f, 0.3f, -0.7f * j - 0.5f));
					M = T * TransZ * S_piece;
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
					glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
					rook.draw(prog);
				}
				if (board[j][i][1] == 'N') {
					S_piece = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
					T = glm::translate(glm::mat4(1.0f), glm::vec3(i * 0.7f - 2.5f, 0.3f, -0.7f * j - 0.5f));
					M = T * TransZ * S_piece;
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
					glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
					knight.draw(prog);
				}
				if (board[j][i][1] == 'B') {
					bishop.draw(prog);
				}
				if (board[j][i][1] == 'Q') {
					queen.draw(prog);
				}
				if (board[j][i][1] == 'K') {
					king.draw(prog);
				}
				if (board[j][i][1] == 'P') {
					S_piece = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 0.3f));
					T = glm::translate(glm::mat4(1.0f), glm::vec3(i * 0.7f - 2.5f, 0.3f, -0.7f * j - 0.5f));
					M = T * TransZ * S_piece;
					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
					glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
					glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
					glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
					pawn.draw(prog);
				}

				if (board[j][i][0] == 'B') {
					prog2->unbind();
				}
				else if (board[j][i][0] == 'W') {
					prog->unbind();
				}
			}
		}

		prog3->bind();
		//highlighting box
		glm::mat4 S_box = glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.05f, 0.35f));
		glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(selectedSquare[0] * 0.7f - 2.5f, 0.1f, -0.7f * selectedSquare[1] - 0.5f));
		M = T * TransZ * S_box;

		//send the matrices to the shaders
		glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog3->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog3->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog3->getUniform("campos"), 1, &mycam.pos[0]);

		glBindVertexArray(VertexArrayID);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);

		prog3->unbind();

		// available moves

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'P') {
			valid_moves = valid_pawn_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'N') {
			valid_moves = valid_knight_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'B') {
			valid_moves = valid_bishop_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'R') {
			valid_moves = valid_rook_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'Q') {
			valid_moves = valid_queen_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		if (pieceSelected && board[pieceToMove[1]][pieceToMove[0]][1] == 'K') {
			valid_moves = valid_king_moves(current_player, pieceToMove[0], pieceToMove[1]);
		}

		//check for checks
		int i = 0;
		while (i < valid_moves.size()) {
			//remove moves such that the king is still in check
			calcBoard[valid_moves[i][1]][valid_moves[i][0]] = calcBoard[pieceToMove[1]][pieceToMove[0]];
			calcBoard[pieceToMove[1]][pieceToMove[0]] = "OO";
			if (king_in_check(current_player)) {
				valid_moves.erase(valid_moves.begin() + i);
			}
			else {
				i++;
			}
			for (int x = 0; x < 8; x++) {
				for (int y = 0; y < 8; y++) {
					calcBoard[x][y] = board[x][y];
				}
			}
		}

		prog4->bind();
		for (int i = 0; i < valid_moves.size(); i++) {
			glm::mat4 S_box = glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.05f, 0.35f));
			glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(valid_moves[i][0] * 0.7f - 2.5f, 0.1f, -0.7f * valid_moves[i][1] - 0.5f));
			M = T * TransZ * S_box;

			//send the matrices to the shaders
			glUniformMatrix4fv(prog3->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog3->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(prog3->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniform3fv(prog3->getUniform("campos"), 1, &mycam.pos[0]);

			glBindVertexArray(VertexArrayID);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
		}
		
		prog4->unbind();
	}
};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
