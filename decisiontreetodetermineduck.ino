// Node structure for binary decision tree
struct TreeNode {
  String question;                // question to ask at this node
  String species;                 // non-empty only for leaf nodes
  TreeNode* yesBranch = nullptr; // left
  TreeNode* noBranch = nullptr;  // right

  TreeNode(String q, String s = "") {
    question = q;
    species = s;
  }
};

// Tree root node
TreeNode* buildDecisionTree() {
  // Leaf nodes
  TreeNode* wibbo = new TreeNode("", "Wibbo");
  TreeNode* gribbit = new TreeNode("", "Gribbit");
  TreeNode* snorkle = new TreeNode("", "Snorkle");
  TreeNode* zapple = new TreeNode("", "Zapple");
  TreeNode* unknown = new TreeNode("", "Unknown or Uncertain");

  // Second level
  TreeNode* ir457 = new TreeNode("IR≈457Hz?", "");
  TreeNode* rad100 = new TreeNode("RAD≈100Hz?", "");
  TreeNode* ir293 = new TreeNode("IR≈293Hz?", "");
  TreeNode* rad150 = new TreeNode("RAD≈150Hz?", "");

  ir457->yesBranch = wibbo;
  ir457->noBranch = rad100;

  rad100->yesBranch = gribbit;
  rad100->noBranch = unknown;

  ir293->yesBranch = snorkle;
  ir293->noBranch = rad150;

  rad150->yesBranch = zapple;
  rad150->noBranch = unknown;

  // Root
  TreeNode* magnetic = new TreeNode("Magnetic==Down?", "");
  magnetic->yesBranch = ir457;
  magnetic->noBranch = ir293;

  return magnetic;
}

// Utility function for fuzzy match
bool approxEquals(float value, float target, float tolerance = 10.0) {
  return abs(value - target) <= tolerance;
}

// Recursive evaluation of the tree
String evaluateTree(TreeNode* node, float ir, float rad, String mag) {
  if (node->species != "") {
    return node->species;
  }

  if (node->question == "Magnetic==Down?") {
    return evaluateTree((mag == "Down") ? node->yesBranch : node->noBranch, ir, rad, mag);
  }

  if (node->question == "IR≈457Hz?") {
    return evaluateTree(approxEquals(ir, 457.0) ? node->yesBranch : node->noBranch, ir, rad, mag);
  }

  if (node->question == "RAD≈100Hz?") {
    return evaluateTree(approxEquals(rad, 100.0) ? node->yesBranch : node->noBranch, ir, rad, mag);
  }

  if (node->question == "IR≈293Hz?") {
    return evaluateTree(approxEquals(ir, 293.0) ? node->yesBranch : node->noBranch, ir, rad, mag);
  }

  if (node->question == "RAD≈150Hz?") {
    return evaluateTree(approxEquals(rad, 150.0) ? node->yesBranch : node->noBranch, ir, rad, mag);
  }

  return "Evaluation Error";
}

void setup() {
  Serial.begin(9600);

  // Simulated sensor data
  float infrared = 292.5;
  float radio = 148.0;
  String magnetic = "Up";

  TreeNode* root = buildDecisionTree();
  String result = evaluateTree(root, infrared, radio, magnetic);
  Serial.println("Duck Species: " + result);
}

void loop() {
  // Add repeated sensor polling here if needed
}
