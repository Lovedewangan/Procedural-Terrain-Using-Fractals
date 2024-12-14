#include <freeglut.h>
#include <cmath>
#include <vector>
#include <random>
#include <iostream>
#include <algorithm>  

#define M_PI 3.14159265358979323846

float cameraPosX = 0.0f;
float cameraPosY = 0.0f;
float cameraPosZ = 100.0f;  

float cameraYaw = 0.8f;    
float cameraPitch = 0.0f;  


class AtmosphericRenderer {
private:
    float timeOfDay;  
    GLfloat skyColor[4];
    GLfloat dynamicLightPosition[4];
    GLfloat dynamicAmbientLight[4];
    GLfloat dynamicDiffuseLight[4];

    // New star-related members
    struct Star {
        float x, y, z;  // 3D position
        float brightness;
        float twinkleOffset;  // For slight brightness variations
    };
    std::vector<Star> stars;
    const int NUM_STARS = 500;  // Adjust for desired star density

    void generateStars() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> posDistr(-1.0, 1.0);
        std::uniform_real_distribution<> brightDistr(0.3, 1.0);

        stars.clear();
        stars.resize(NUM_STARS);

        for (auto& star : stars) {
            // Generate star positions on a sphere
            float theta = posDistr(gen) * M_PI;
            float phi = posDistr(gen) * 2 * M_PI;

            star.x = std::sin(theta) * std::cos(phi);
            star.y = std::sin(theta) * std::sin(phi);
            star.z = std::cos(theta);

            // Normalize to unit sphere
            float length = std::sqrt(star.x * star.x + star.y * star.y + star.z * star.z);
            star.x /= length;
            star.y /= length;
            star.z /= length;

            // Initial brightness and twinkle offset
            star.brightness = brightDistr(gen);
            star.twinkleOffset = posDistr(gen);
        }
    }

    float calculateStarBrightness(float baseTime) {
        // Only show stars during night
        if (baseTime >= 0 && baseTime < 6) {
            // Gradually fade in/out stars near dawn/dusk
            float fadeFactor = 1.0f;
            if (baseTime >= 0 && baseTime < 1) {
                // Fade in
                fadeFactor = baseTime / 1.0f;
            }
            else if (baseTime >= 5 && baseTime < 6) {
                // Fade out
                fadeFactor = 1.0f - (baseTime - 5.0f);
            }
            return fadeFactor;
        }
        else if (baseTime >= 18 && baseTime < 24) {
            // Evening transition
            float fadeFactor = 1.0f;
            if (baseTime >= 18 && baseTime < 19) {
                // Fade in
                fadeFactor = (baseTime - 18.0f);
            }
            else if (baseTime >= 23 && baseTime < 24) {
                // Fade out
                fadeFactor = 1.0f - (baseTime - 23.0f);
            }
            return fadeFactor;
        }
        return 0.0f;
    }

    void renderStars() {
        float starBrightness = calculateStarBrightness(timeOfDay);

        if (starBrightness <= 0) return;

        glPushMatrix();
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        // Rotate stars with camera to create immersive sky effect
        glRotatef(cameraYaw * 180.0f / M_PI, 0, 0, 1);
        glRotatef(cameraPitch * 180.0f / M_PI, 1, 0, 0);

        glBegin(GL_POINTS);
        for (const auto& star : stars) {
            // Slight twinkle effect using sine wave
            float twinkleFactor = std::sin(timeOfDay * 2 + star.twinkleOffset) * 0.2f + 1.0f;

            glColor4f(1.0f, 1.0f, 1.0f,
                star.brightness * starBrightness * twinkleFactor * 0.8f);

            // Render star slightly in front of sky
            glVertex3f(star.x * 0.99, star.y * 0.99, star.z * 0.99);
        }
        glEnd();

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glPopMatrix();
    }

public:
    AtmosphericRenderer() : timeOfDay(12.0f) {
        updateAtmosphericConditions();
        generateStars();
    }

    void updateTime(float deltaTime) {
        timeOfDay += deltaTime * 2.0f;
        if (timeOfDay >= 24.0f) timeOfDay = 0.0f;
        updateAtmosphericConditions();
    }

    void updateAtmosphericConditions() {
        // Sky color transitions
        if (timeOfDay >= 0 && timeOfDay < 6) {  // Night
            skyColor[0] = 0.1f; skyColor[1] = 0.1f; skyColor[2] = 0.2f;
            dynamicLightPosition[0] = -0.5f;
            dynamicLightPosition[1] = -0.5f;
            dynamicLightPosition[2] = -1.0f;
        }
        else if (timeOfDay >= 6 && timeOfDay < 8) {  // Dawn
            float t = (timeOfDay - 6) / 2.0f;
            skyColor[0] = 0.6f + 0.4f * t;
            skyColor[1] = 0.7f + 0.3f * t;
            skyColor[2] = 0.8f - 0.3f * t;
            dynamicLightPosition[0] = 0.5f * (1 - t);
            dynamicLightPosition[1] = 0.5f * (1 - t);
            dynamicLightPosition[2] = 1.0f * t;
        }
        else if (timeOfDay >= 8 && timeOfDay < 16) {  // Day
            skyColor[0] = 0.6f; skyColor[1] = 0.7f; skyColor[2] = 0.8f;
            dynamicLightPosition[0] = 0.5f;
            dynamicLightPosition[1] = 0.5f;
            dynamicLightPosition[2] = 1.0f;
        }
        else if (timeOfDay >= 16 && timeOfDay < 18) {  // Sunset
            float t = (timeOfDay - 16) / 2.0f;
            skyColor[0] = 0.6f - 0.5f * t;
            skyColor[1] = 0.7f - 0.6f * t;
            skyColor[2] = 0.8f - 0.6f * t;
            dynamicLightPosition[0] = 0.5f * (1 - t);
            dynamicLightPosition[1] = 0.5f * (1 - t);
            dynamicLightPosition[2] = 1.0f * (1 - t);
        }
        else {  // Night
            skyColor[0] = 0.1f; skyColor[1] = 0.1f; skyColor[2] = 0.2f;
            dynamicLightPosition[0] = -0.5f;
            dynamicLightPosition[1] = -0.5f;
            dynamicLightPosition[2] = -1.0f;
        }

        skyColor[3] = 1.0f;
        dynamicLightPosition[3] = 0.0f;

        // Adjust light colors based on time
        dynamicAmbientLight[0] = skyColor[0] * 0.3f;
        dynamicAmbientLight[1] = skyColor[1] * 0.3f;
        dynamicAmbientLight[2] = skyColor[2] * 0.3f;
        dynamicAmbientLight[3] = 1.0f;

        dynamicDiffuseLight[0] = skyColor[0];
        dynamicDiffuseLight[1] = skyColor[1];
        dynamicDiffuseLight[2] = skyColor[2];
        dynamicDiffuseLight[3] = 1.0f;
    }

    void applySkyAndLighting() {
        glClearColor(skyColor[0], skyColor[1], skyColor[2], skyColor[3]);

        glLightfv(GL_LIGHT0, GL_POSITION, dynamicLightPosition);
        glLightfv(GL_LIGHT0, GL_AMBIENT, dynamicAmbientLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, dynamicDiffuseLight);

        // Render stars during night
        renderStars();
    }


    float getTimeOfDay() const { return timeOfDay; }
};

class CloudGenerator {
private:
    std::vector<std::vector<float>> cloudDensityMap;
    int resolution;
    unsigned int chunkSeed;

    float cloudNoise(float x, float y, int octaves = 4) {
        std::mt19937 rng(chunkSeed);
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        float noise = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            float sampleX = x * frequency;
            float sampleY = y * frequency;

            
            float randomOffset = dist(rng) * 0.1f;
            noise += amplitude * (std::sin(sampleX + randomOffset) * std::cos(sampleY + randomOffset));

            maxValue += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return noise / maxValue;
    }

public:
    CloudGenerator(int res = 256, unsigned int seed = 12345)
        : resolution(res), chunkSeed(seed) {
        cloudDensityMap.resize(resolution, std::vector<float>(resolution, 0.0f));
        generateClouds();
    }

    void regenerateClouds(unsigned int newSeed) {
        chunkSeed = newSeed;
        generateClouds();
    }

    void generateClouds() {
        for (int x = 0; x < resolution; ++x) {
            for (int y = 0; y < resolution; ++y) {
                float noiseValue = cloudNoise(x / 128.0f, y / 128.0f);
                cloudDensityMap[x][y] = std::max(0.0f, std::min(1.0f, noiseValue));
            }
        }
    }

    void renderClouds(float offsetX = 0, float offsetY = 0, float height = 50.0f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBegin(GL_QUADS);
        for (int x = 0; x < resolution - 1; ++x) {
            for (int y = 0; y < resolution - 1; ++y) {
                float density = cloudDensityMap[x][y];

                if (density > 0.5f) {  

                    float opacity = std::min(1.0f, (density - 0.5f) * 1.2f);

                    
                    float colorVar = 0.1f * (1.0f - density);
                    glColor4f(1.0f - colorVar, 1.0f - colorVar, 1.0f - colorVar, opacity);

                    glVertex3f(x + offsetX, y + offsetY, height);
                    glVertex3f(x + 1 + offsetX, y + offsetY, height);
                    glVertex3f(x + 1 + offsetX, y + 1 + offsetY, height);
                    glVertex3f(x + offsetX, y + 1 + offsetY, height);

                }
            }
        }
        glEnd();

        glDisable(GL_BLEND);
    }
};

class ChunkGenerator {
private:
    int chunkSize;
    float roughness;
    unsigned int baseSeed;
    std::mt19937 rng;

    std::vector<std::vector<float>> heightMap;

    float displace(float size) {
        static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        return dist(rng) * size;
    }

    void diamondSquareAlgorithm(unsigned int chunkSeed) {
        int width = chunkSize + 1;
        heightMap.resize(width, std::vector<float>(width, 0.0f));

        rng.seed(chunkSeed);

        
        heightMap[0][0] = 0.2f + displace(0.1f);
        heightMap[0][width - 1] = 0.3f + displace(0.1f);
        heightMap[width - 1][0] = 0.1f + displace(0.1f);
        heightMap[width - 1][width - 1] = 0.4f + displace(0.1f);


        
        float size = width - 1;
        float h = roughness * 1.2f;

        while (size >= 1) {
            
            /*
            This divides the grid into Smaller Diamond Steps and it 
            calculates the center point of each division.
            */
            
            for (int x = 0; x < width - 1; x += size) {
                for (int y = 0; y < width - 1; y += size) {
                    int midX = x + size / 2;
                    int midY = y + size / 2;

                    float avg = (
                        heightMap[x][y] +
                        heightMap[x + size][y] +
                        heightMap[x][y + size] +
                        heightMap[x + size][y + size]
                        ) / 4.0f;

                    float variation = displace(h) * (1.3f + std::abs(avg - 0.5f) * 1.5f);
                    heightMap[midX][midY] = std::min(1.0f, std::max(0.0f, avg + variation));
                }
            }

            // Square step
            for (int x = 0; x < width - 1; x += size) {
                for (int y = 0; y < width - 1; y += size) {
                    int midX = x + size / 2;
                    int midY = y + size / 2;

                    if (x > 0) {

                        float avg = (
                            heightMap[x][y] +
                            heightMap[x][y + size] +
                            heightMap[midX][midY] +
                            heightMap[x - size / 2][midY]
                            ) / 4.0f;

                        heightMap[x][midY] = std::min(1.0f, std::max(0.0f,
                            avg + displace(h) * (1.3f + std::abs(avg - 0.5f) * 1.5f)));
                    }

                    if (y > 0) {

                        float avg = (
                            heightMap[x][y] +
                            heightMap[x + size][y] +
                            heightMap[midX][midY] +
                            heightMap[midX][y - size / 2]
                            ) / 4.0f;

                        heightMap[midX][y] = std::min(1.0f, std::max(0.0f,
                            avg + displace(h) * (1.3f + std::abs(avg - 0.5f) * 1.5f)));
                    }
                }
            }

            size /= 2;
            h *= 0.55f;
        }

        // Smooth peaks
        smoothPeaks();
    }

    void smoothPeaks() {
        std::vector<std::vector<float>> smoothedHeightMap = heightMap;
        int width = heightMap.size();

        for (int x = 1; x < width - 1; ++x) {

            for (int y = 1; y < width - 1; ++y) {

                float smoothedHeight = (
                    heightMap[x - 1][y - 1] * 0.05f +
                    heightMap[x - 1][y] * 0.1f +
                    heightMap[x - 1][y + 1] * 0.05f +
                    heightMap[x][y - 1] * 0.1f +
                    heightMap[x][y] * 0.4f +
                    heightMap[x][y + 1] * 0.1f +
                    heightMap[x + 1][y - 1] * 0.05f +
                    heightMap[x + 1][y] * 0.1f +
                    heightMap[x + 1][y + 1] * 0.05f
                    );

                smoothedHeightMap[x][y] = std::pow(smoothedHeight, 0.78f);
            }
        }

        heightMap = smoothedHeightMap;
    }

    
    float fractalNoise(float x, float y, float persistence = 0.5f, int octaves = 6) {
        float total = 0.0f;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; ++i) {
            
            float sampleX = x * frequency;
            float sampleY = y * frequency;

            
            float noise = 0.5f * (std::sin(sampleX) + std::cos(sampleY));

            total += noise * amplitude;
            maxValue += amplitude;

            amplitude *= persistence;
            frequency *= 2.0f;
        }

        return total / maxValue;
    }

    void addErosionSimulation() {
        int width = heightMap.size();
        std::vector<std::vector<float>> erosionMap = heightMap;

       
        for (int iteration = 0; iteration < 10; ++iteration) {
            for (int x = 1; x < width - 1; ++x) {
                for (int y = 1; y < width - 1; ++y) {
                    

                    float currentHeight = heightMap[x][y];
                    float maxDropHeight = 0;
                    int dropX = x, dropY = y;

                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            float neighborHeight = heightMap[x + dx][y + dy];
                            float dropHeight = currentHeight - neighborHeight;

                            if (dropHeight > maxDropHeight) {
                                maxDropHeight = dropHeight;
                                dropX = x + dx;
                                dropY = y + dy;
                            }
                        }
                    }

                    
                    if (dropX != x || dropY != y) {
                        float sedimentAmount = maxDropHeight * 0.1f;
                        erosionMap[x][y] -= sedimentAmount;
                        erosionMap[dropX][dropY] += sedimentAmount;
                    }
                }
            }


            heightMap = erosionMap;
        }
    }

    void applyBiomeVariation() {
        int width = heightMap.size();

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < width; ++y) {
                
                float terrainNoise = fractalNoise(x / 256.0f, y / 256.0f);
                float biomeNoise = fractalNoise(x / 128.0f, y / 128.0f, 0.6f, 4);

                
                heightMap[x][y] = std::min(1.0f, std::max(0.0f,
                    heightMap[x][y] +
                    terrainNoise * 0.2f +
                    biomeNoise * 0.1f
                ));
            }
        }
    }

    
    void getTerrainColor(float height, float& r, float& g, float& b)  {
        
        if (height < 0.1f) {
            
            r = 0.0f; g = 0.1f; b = 0.4f;
        }
        else if (height < 0.2f) {
            
            r = 0.1f; g = 0.3f; b = 0.5f;
        }
        else if (height < 0.3f) {
            
            r = 0.85f; g = 0.8f; b = 0.6f;
        }
        else if (height < 0.45f) {
            
            r = 0.2f; g = 0.5f; b = 0.2f;
        }
        else if (height < 0.6f) {
            
            r = 0.4f; g = 0.6f; b = 0.3f;
        }
        else if (height < 0.75f) {
            
            r = 0.5f; g = 0.5f; b = 0.5f;
        }
        else if (height < 0.9f) {
            
            r = 0.6f; g = 0.6f; b = 0.6f;
        }
        else {
            
            r = 0.9f; g = 0.9f; b = 1.0f;
        }

        
        float localVariation = fractalNoise(r, g, 0.5f, 3);
        r += localVariation * 0.1f;
        g += localVariation * 0.1f;
        b += localVariation * 0.1f;
    }

public:
    ChunkGenerator(int size = 128, float rough = 0.82f)
        : chunkSize(size), roughness(rough), baseSeed(12345) {}

    void generateChunk(unsigned int chunkSeed) {
        diamondSquareAlgorithm(chunkSeed);

        addErosionSimulation();
        applyBiomeVariation();

        
        smoothPeaks();
    }

    void render(float offsetX = 0, float offsetY = 0) {
        float scale = 90.0f;
        int width = heightMap.size();

        glBegin(GL_TRIANGLES);
        for (int x = 0; x < width - 1; ++x) {
            for (int y = 0; y < width - 1; ++y) {
                float h1 = std::pow(heightMap[x][y], 1.5f) * scale;
                float h2 = std::pow(heightMap[x + 1][y], 1.5f) * scale;
                float h3 = std::pow(heightMap[x][y + 1], 1.5f) * scale;
                float h4 = std::pow(heightMap[x + 1][y + 1], 1.5f) * scale;

                float r1, g1, b1, r2, g2, b2, r3, g3, b3, r4, g4, b4;
                getTerrainColor(heightMap[x][y], r1, g1, b1);
                getTerrainColor(heightMap[x + 1][y], r2, g2, b2);
                getTerrainColor(heightMap[x][y + 1], r3, g3, b3);
                getTerrainColor(heightMap[x + 1][y + 1], r4, g4, b4);

                // First triangle
                glColor3f(r1, g1, b1);
                glVertex3f(x + offsetX, y + offsetY, h1);
                glColor3f(r2, g2, b2);
                glVertex3f(x + 1 + offsetX, y + offsetY, h2);
                glColor3f(r3, g3, b3);
                glVertex3f(x + offsetX, y + 1 + offsetY, h3);

                // Second triangle
                glColor3f(r2, g2, b2);
                glVertex3f(x + 1 + offsetX, y + offsetY, h2);
                glColor3f(r4, g4, b4);
                glVertex3f(x + 1 + offsetX, y + 1 + offsetY, h4);
                glColor3f(r3, g3, b3);
                glVertex3f(x + offsetX, y + 1 + offsetY, h3);
            }
        }
        glEnd();
    }

    float getMaxHeight() const {
        float maxHeight = 0.0f;
        for (const auto& row : heightMap) {
            for (float height : row) {
                maxHeight = std::max(maxHeight, height);
            }
        }
        return maxHeight * 90.0f;
    }
};

// Global variables
const int MAX_CHUNKS = 9;  // 3x3 grid of chunks
const int CHUNK_SIZE = 256;
const float MOVE_SPEED = 0.5f;
const float MAX_FORWARD_DISTANCE = CHUNK_SIZE * 3.0f;  // Limit movement to 3 chunk sizes

class TerrainManager {
private:
    std::vector<std::vector<ChunkGenerator>> chunks;
    std::vector<std::vector<CloudGenerator>> chunkClouds;
    float currentOffset;
    unsigned int baseSeed;
    std::vector<std::vector<float>> heightMap;
    bool cloudRenderingEnabled;

public:
    TerrainManager(unsigned int seed = 12345)
        : currentOffset(0),
        baseSeed(seed),
        cloudRenderingEnabled(true)  // Default to rendering clouds
    {
        // Resize both terrain and cloud generators
        chunks.resize(3, std::vector<ChunkGenerator>(3, ChunkGenerator(CHUNK_SIZE)));
        chunkClouds.resize(3, std::vector<CloudGenerator>(3, CloudGenerator(CHUNK_SIZE)));

        // Initial chunk and cloud generation
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                unsigned int chunkSeed = baseSeed + x * 3 + y;
                chunks[x][y].generateChunk(chunkSeed);
                chunkClouds[x][y].regenerateClouds(chunkSeed);
            }
        }
    }


    void moveForward(float distance) {
        currentOffset = std::min(currentOffset + distance, MAX_FORWARD_DISTANCE);
    }

    void moveBackward(float distance) {
        currentOffset = std::max(currentOffset - distance, 0.0f);
    }

    void toggleCloudRendering() {
        cloudRenderingEnabled = !cloudRenderingEnabled;
    }

    void render() {
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                // Remove the chunk spacing, align chunks exactly
                float xOffset = x * CHUNK_SIZE;
                float yOffset = y * CHUNK_SIZE - currentOffset;

                // Render terrain
                chunks[x][y].render(xOffset, yOffset);

                // If clouds are enabled, render clouds for this chunk
                if (cloudRenderingEnabled) {
                    float cloudHeight = chunks[x][y].getMaxHeight() + 50.0f;
                    chunkClouds[x][y].renderClouds(xOffset, yOffset, cloudHeight);
                }
            }
        }
    }

    float getMaxHeight() const {
        float maxHeight = 0.0f;
        for (const auto& row : heightMap) {
            for (float height : row) {
                maxHeight = std::max(maxHeight, height);
            }
        }
        return maxHeight * 90.0f;
    }

    float getCurrentOffset() const { return currentOffset; }
    float getMaxForwardDistance() const { return MAX_FORWARD_DISTANCE; }
};



GLfloat lightPosition[4] = { 0.5f, 0.5f, 1.0f, 0.0f };
GLfloat ambientLight[4] = { 0.3f, 0.3f, 0.3f, 1.0f };   
GLfloat diffuseLight[4] = { 1.0f, 0.95f, 0.85f, 1.0f }; 
GLfloat specularLight[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  

void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // Set light colors with increased intensity
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    
    GLfloat fogColor[4] = { 0.6f, 0.6f, 0.7f, 0.7f };

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.0025f);  
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 200.0f);
}
// Global variables
TerrainManager* terrainManager = nullptr;



// Mouse tracking variables
int lastMouseX = 0;
int lastMouseY = 0;
bool rightMouseButtonDown = false;  


AtmosphericRenderer* atmosphericRenderer = nullptr;
CloudGenerator* cloudGenerator = nullptr;

bool renderClouds = true;

void renderBitmapString(float x, float y, void* font, const char* string) {
    glColor3f(1.0f, 1.0f, 1.0f);  // White text color
    glRasterPos2f(x, y);

    while (*string) {
        glutBitmapCharacter(font, *string);
        string++;
    }
}

void displayInstructions() {
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    gluOrtho2D(0, width, 0, height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    
    glDisable(GL_DEPTH_TEST);

    
    float startY = height - 20;  
    void* font = GLUT_BITMAP_HELVETICA_12;

    renderBitmapString(10, startY, font, "Key Commands:");
    renderBitmapString(10, startY - 20, font, "W/S: Move Forward/Backward");
    renderBitmapString(10, startY - 40, font, "A/D: Strafe Left/Right");
    renderBitmapString(10, startY - 60, font, "Q/E: Move Up/Down");
    renderBitmapString(10, startY - 80, font, "Right Mouse: Look Around");
    renderBitmapString(10, startY - 100, font, "T/t: Advance/Rewind Time");
    renderBitmapString(10, startY - 120, font, "C: Toggle Cloud Rendering");
    renderBitmapString(10, startY - 140, font, "ESC: Exit");
    renderBitmapString(1530, 20, font, "Love Dewangan 500109339");

    // Restore previous states
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    atmosphericRenderer->applySkyAndLighting();

    // Calculate look-at point based on camera orientation
    float lookAtX = cameraPosX + cos(cameraYaw) * cos(cameraPitch);
    float lookAtY = cameraPosY + sin(cameraYaw) * cos(cameraPitch);
    float lookAtZ = cameraPosZ + sin(cameraPitch);

    gluLookAt(
        cameraPosX, cameraPosY, cameraPosZ,  // Camera position
        lookAtX, lookAtY, lookAtZ,           // Look-at point
        0.0f, 0.0f, 1.0f                    // Up vector
    );

    glEnable(GL_FOG);  

    terrainManager->render();

    glDisable(GL_FOG);

    if (renderClouds) {
        cloudGenerator->renderClouds(0, 0, terrainManager->getMaxHeight() + 50.0f);
    }

    
    displayInstructions();

    glutSwapBuffers();
}


void mouseMotion(int x, int y) {
    
    if (rightMouseButtonDown) {


        float deltaX = -(x - lastMouseX);  
        float deltaY = (y - lastMouseY);  

        
        cameraYaw += deltaX * 0.005f;    
        cameraPitch -= deltaY * 0.005f;  

        
        float minPitch = -M_PI / 2.1f;
        float maxPitch = M_PI / 2.1f;

        cameraPitch = (cameraPitch < minPitch) ? minPitch :
            (cameraPitch > maxPitch) ? maxPitch :
            cameraPitch;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void mouseButton(int button, int state, int x, int y) {
    
    if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) {
            rightMouseButtonDown = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP) {
            rightMouseButtonDown = false;
        }
    }
    
    else if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            
            std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
        }
    }
}

void keyboard(unsigned char key, int x, int y) {
    float moveSpeed = 7.0f;

    // Calculate forward and right vectors
    float forwardX = cos(cameraYaw) * cos(cameraPitch);
    float forwardY = sin(cameraYaw) * cos(cameraPitch);
    float rightX = sin(cameraYaw);
    float rightY = -cos(cameraYaw);

    switch (key) {
    case 'w':  
        cameraPosX += forwardX * moveSpeed;
        cameraPosY += forwardY * moveSpeed;
        terrainManager->moveForward(MOVE_SPEED);
        break;

    case 's':  
        cameraPosX -= forwardX * moveSpeed;
        cameraPosY -= forwardY * moveSpeed;
        terrainManager->moveBackward(MOVE_SPEED);
        break;

    case 'a':  
        cameraPosX -= rightX * moveSpeed;
        cameraPosY -= rightY * moveSpeed;
        break;

    case 'd':  
        cameraPosX += rightX * moveSpeed;
        cameraPosY += rightY * moveSpeed;
        break;

    case 'q':  
        cameraPosZ += moveSpeed;
        break;

    case 'e':  
        cameraPosZ -= moveSpeed;
        break;
        
    case 't':  
        atmosphericRenderer->updateTime(1.0f);
        break;

    case 'T':  
        atmosphericRenderer->updateTime(-1.0f);
        break;

    case 'c':  
        terrainManager->toggleCloudRendering();
        std::cout << "Clouds " << (renderClouds ? "enabled" : "disabled") << std::endl;
        break;

    case 27:
        exit(0);
        break;
    }

    glutPostRedisplay();
}
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)w / (float)h, 0.1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("Dynamic Terrain Generation");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.7f, 0.8f, 1.0f);  // Sky color

    
    terrainManager = new TerrainManager();
    
    atmosphericRenderer = new AtmosphericRenderer();
    cloudGenerator = new CloudGenerator();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    setupLighting();

    glutMainLoop();

    delete terrainManager;
    return 0;
}