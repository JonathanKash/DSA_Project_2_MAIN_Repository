#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

#include "Node.h"
#include "AdjacencyGrid.h"
#include "QuadTree.h"

// map 'S','E','I','R' to distinct colors for rendering
static sf::Color colorForState(char s) {
    if (s == 'S') return sf::Color(100, 100, 255);   // blue
    if (s == 'E') return sf::Color(255, 165, 0);     // orange
    if (s == 'I') return sf::Color(220, 20, 60);     // red
    if (s == 'R') return sf::Color(60, 179, 113);    // green
    return sf::Color::White;
}

//a single-line input for numeric text (digits, '.', '-')

class TextBox {
public:
    TextBox() : focused_(false) {}

    //geometry, labels, and placeholder text
    void init(const sf::Font& font,
              const std::string& label,
              float x, float y, float w, float h,
              const std::string& placeholder) {
        label_.setFont(font);
        label_.setString(label);
        label_.setCharacterSize(18);
        label_.setFillColor(sf::Color::White);
        label_.setPosition(x, y - 24.0f);

        box_.setPosition(x, y);
        box_.setSize(sf::Vector2f(w, h));
        box_.setFillColor(sf::Color(30, 30, 30));
        box_.setOutlineThickness(1.0f);
        box_.setOutlineColor(sf::Color(80, 80, 80));

        text_.setFont(font);
        text_.setCharacterSize(18);
        text_.setFillColor(sf::Color(220, 220, 220));
        text_.setPosition(x + 8.0f, y + 6.0f);

        placeholder_.setFont(font);
        placeholder_.setString(placeholder);
        placeholder_.setCharacterSize(18);
        placeholder_.setFillColor(sf::Color(130, 130, 130));
        placeholder_.setPosition(x + 8.0f, y + 6.0f);
    }

    void setString(const std::string& s) { value_ = s; }
    const std::string& getString() const { return value_; }

    //indicate focus and remember state
    void setFocused(bool f) {
        focused_ = f;
        box_.setOutlineColor(focused_ ? sf::Color(180, 180, 180) : sf::Color(80, 80, 80));
    }

    //test for mouse clicks
    bool contains(int mx, int my) const {
        sf::FloatRect r = box_.getGlobalBounds();
        return r.contains((float)mx, (float)my);
    }

    // Handle text events
    void handleTextEntered(sf::Uint32 unicode) {
        if (!focused_) return;
        if (unicode == 13) { setFocused(false); return; }       // Enter
        if (unicode == 8)  { if (!value_.empty()) value_.pop_back(); return; } // Backspace
        if ((unicode >= '0' && unicode <= '9') || unicode == '.' || unicode == '-') {
            value_.push_back((char)unicode);
        }
    }

    // Draw the box
    void draw(sf::RenderTarget& rt) {
        rt.draw(box_);
        if (value_.empty() && !focused_) rt.draw(placeholder_);
        else { text_.setString(value_); rt.draw(text_); }
        rt.draw(label_);
    }

private:
    sf::RectangleShape box_;
    sf::Text label_;
    sf::Text text_;
    sf::Text placeholder_;
    std::string value_;
    bool focused_;
};

//toggle, the selected one gets a brighter outline
class Toggle {
public:
    Toggle() : index_(0) {}

    void init(const sf::Font& font, float x, float y) {
        label_.setFont(font);
        label_.setString("Representation");
        label_.setCharacterSize(18);
        label_.setFillColor(sf::Color::White);
        label_.setPosition(x, y - 24.0f);

        gridBtn_.setPosition(x, y);
        gridBtn_.setSize(sf::Vector2f(130.0f, 34.0f));
        gridBtn_.setFillColor(sf::Color(40, 40, 40));
        gridBtn_.setOutlineThickness(1.0f);
        gridBtn_.setOutlineColor(sf::Color(80, 80, 80));

        quadBtn_.setPosition(x + 140.0f, y);
        quadBtn_.setSize(sf::Vector2f(130.0f, 34.0f));
        quadBtn_.setFillColor(sf::Color(40, 40, 40));
        quadBtn_.setOutlineThickness(1.0f);
        quadBtn_.setOutlineColor(sf::Color(80, 80, 80));

        gridText_.setFont(font);
        gridText_.setString("Grid");
        gridText_.setCharacterSize(18);
        gridText_.setFillColor(sf::Color::White);
        gridText_.setPosition(x + 48.0f, y + 6.0f);

        quadText_.setFont(font);
        quadText_.setString("Quadtree");
        quadText_.setCharacterSize(18);
        quadText_.setFillColor(sf::Color::White);
        quadText_.setPosition(x + 140.0f + 24.0f, y + 6.0f);

        updateHighlight();
    }

    //click switches selection if within either button
    void onClick(int mx, int my) {
        if (gridBtn_.getGlobalBounds().contains((float)mx, (float)my)) index_ = 0;
        else if (quadBtn_.getGlobalBounds().contains((float)mx, (float)my)) index_ = 1;
        updateHighlight();
    }

    int getIndex() const { return index_; } // 0=Grid, 1=Quadtree

    void draw(sf::RenderTarget& rt) {
        rt.draw(gridBtn_); rt.draw(quadBtn_);
        rt.draw(gridText_); rt.draw(quadText_);
        rt.draw(label_);
    }

private:
    void updateHighlight() {
        gridBtn_.setOutlineColor(index_ == 0 ? sf::Color(180,180,180) : sf::Color(80,80,80));
        quadBtn_.setOutlineColor(index_ == 1 ? sf::Color(180,180,180) : sf::Color(80,80,80));
    }

    sf::Text label_;
    sf::RectangleShape gridBtn_, quadBtn_;
    sf::Text gridText_, quadText_;
    int index_;
};

//setup UI (text boxes + toggle + start)
//create Grid or QuadtreeSim with provided params
//fixed-interval stepping + drawing nodes
//S/E/I/R counts and controls

int main() {
    // Window and framerate cap
    int winW = 1000, winH = 800;
    sf::RenderWindow window(sf::VideoMode((unsigned int)winW, (unsigned int)winH),
                            "Epidemic Simulator (Setup -> Run)");
    window.setFramerateLimit(60);

    //load font used across UI
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Could not load arial.ttf. Place a TTF font next to the executable.\n";
        return 1;
    }

    //setup controls: alpha/beta/gamma/N/radius
    TextBox tbAlpha, tbBeta, tbGamma, tbN, tbRadius;
    tbAlpha.init(font, "alpha (E -> I / step)",    80.0f, 140.0f, 220.0f, 36.0f, "0.33");
    tbBeta .init(font, "beta  (per I-neighbor)",   80.0f, 220.0f, 220.0f, 36.0f, "0.25");
    tbGamma.init(font, "gamma (I -> R / step)",    80.0f, 300.0f, 220.0f, 36.0f, "0.10");
    tbN    .init(font, "nodes (N)",                80.0f, 380.0f, 220.0f, 36.0f, "400");
    tbRadius.init(font, "radius (Quadtree only)",  80.0f, 460.0f, 220.0f, 36.0f, "3.0");

    //grid/quadtree selector
    Toggle modelToggle;
    modelToggle.init(font, 360.0f, 140.0f);

    //start button
    sf::RectangleShape startBtn(sf::Vector2f(180.0f, 44.0f));
    startBtn.setPosition(360.0f, 220.0f);
    startBtn.setFillColor(sf::Color(70, 120, 90));
    startBtn.setOutlineThickness(1.0f);
    startBtn.setOutlineColor(sf::Color(20, 80, 40));

    sf::Text startText;
    startText.setFont(font);
    startText.setString("Start");
    startText.setCharacterSize(20);
    startText.setFillColor(sf::Color::White);
    startText.setPosition(360.0f + 62.0f, 220.0f + 10.0f);

    //title for setup
    sf::Text title;
    title.setFont(font);
    title.setString("Epidemic SEIR Setup");
    title.setCharacterSize(28);
    title.setFillColor(sf::Color::White);
    title.setPosition(80.0f, 60.0f);

    //setup or running
    enum Screen { Setup, Running };
    Screen screen = Setup;

    //pointers to the selected simulator
    bool usingQuadtree = false;
    QuadtreeSim* qsim = NULL;
    Grid* gsim = NULL;

    //domain in simulation coordinates
    float domainW = 100.0f, domainH = 100.0f;

    //circle primitive for drawing nodes
    float pointRadius = 2.0f;
    sf::CircleShape dot(pointRadius);
    dot.setOrigin(pointRadius, pointRadius);

    //view used in Running screen
    sf::View view; bool viewSet = false;

    //HUD text
    sf::Text hud;
    hud.setFont(font);
    hud.setCharacterSize(16);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(8.0f, 8.0f);

    //simulation pacing
    bool paused = false;
    double stepsPerSecond = 15.0;
    double stepInterval = 1.0 / stepsPerSecond; // real-time interval per sim step
    double simDT = 1.0;                          // model “time” advanced per step
    double acc = 0.0, tSim = 0.0;
    sf::Clock clock;

    //QuadtreeSim::snapshot() output
    std::vector<float> xs, ys;
    std::vector<char>  st;
    std::vector<int>   ids;

    //main loop
    while (window.isOpen()) {
        //inputs
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();

            if (screen == Setup) {
                //handle clicks for focusing inputs / toggling model / starting run
                if (ev.type == sf::Event::MouseButtonPressed) {
                    int mx = ev.mouseButton.x, my = ev.mouseButton.y;

                    //focus whichever textbox was clicked
                    tbAlpha.setFocused(tbAlpha.contains(mx, my));
                    tbBeta .setFocused(tbBeta.contains(mx, my));
                    tbGamma.setFocused(tbGamma.contains(mx, my));
                    tbN    .setFocused(tbN.contains(mx, my));
                    tbRadius.setFocused(tbRadius.contains(mx, my));

                    //toggle model if its buttons were clicked
                    modelToggle.onClick(mx, my);

                    //start button
                    if (startBtn.getGlobalBounds().contains((float)mx, (float)my)) {
                        // Defaults if fields left empty
                        float alpha = 0.33f, beta = 0.25f, gamma = 0.10f, radius = 3.0f;
                        int N = 400;
                        if (!tbAlpha.getString().empty()) alpha = (float)std::atof(tbAlpha.getString().c_str());
                        if (!tbBeta .getString().empty())  beta  = (float)std::atof(tbBeta .getString().c_str());
                        if (!tbGamma.getString().empty())  gamma = (float)std::atof(tbGamma.getString().c_str());
                        if (!tbN    .getString().empty())  N     = std::atoi(tbN    .getString().c_str());
                        if (!tbRadius.getString().empty()) radius = (float)std::atof(tbRadius.getString().c_str());
                        if (N < 1) N = 1;

                        int idx = modelToggle.getIndex();
                        usingQuadtree = (idx == 1);

                        //create the requested simulator and seed 5% infected
                        if (usingQuadtree) {
                            if (qsim) { delete qsim; qsim = NULL; }
                            qsim = new QuadtreeSim(N, domainW, domainH, radius, beta, alpha, gamma, 42u);
                            qsim->seedInfectedPercent(5.0f, true);
                        } else {
                            if (gsim) { delete gsim; gsim = NULL; }
                            gsim = new Grid(N, beta, alpha, gamma, 42u);
                            gsim->seedInfectedPercent(5.0f, true);
                        }

                        //reset timing and transition
                        paused = false; tSim = 0.0; acc = 0.0;
                        clock.restart();
                        screen = Running; viewSet = false;
                    }
                }
                //while focused, numeric textboxes accept typed characters
                else if (ev.type == sf::Event::TextEntered) {
                    tbAlpha.handleTextEntered(ev.text.unicode);
                    tbBeta .handleTextEntered(ev.text.unicode);
                    tbGamma.handleTextEntered(ev.text.unicode);
                    tbN    .handleTextEntered(ev.text.unicode);
                    tbRadius.handleTextEntered(ev.text.unicode);
                }
            } else { // Running
                //keyboard controls: pause, speed, escape to setup, handle resize
                if (ev.type == sf::Event::KeyPressed) {
                    if (ev.key.code == sf::Keyboard::Space) {
                        paused = !paused;
                    } else if (ev.key.code == sf::Keyboard::Add || ev.key.code == sf::Keyboard::Equal) {
                        stepsPerSecond *= 1.25; if (stepsPerSecond > 120.0) stepsPerSecond = 120.0;
                        stepInterval = 1.0 / stepsPerSecond;
                    } else if (ev.key.code == sf::Keyboard::Hyphen || ev.key.code == sf::Keyboard::Dash) {
                        stepsPerSecond /= 1.25; if (stepsPerSecond < 1.0) stepsPerSecond = 1.0;
                        stepInterval = 1.0 / stepsPerSecond;
                    } else if (ev.key.code == sf::Keyboard::Escape) {
                        if (qsim) { delete qsim; qsim = NULL; }
                        if (gsim) { delete gsim; gsim = NULL; }
                        screen = Setup;
                    }
                } else if (ev.type == sf::Event::Resized) {
                    sf::View v = window.getView();
                    v.setSize((float)ev.size.width, (float)ev.size.height);
                    window.setView(v); viewSet = false;
                }
            }
        }

        //draw
        window.clear(sf::Color(18, 18, 18));

        if (screen == Setup) {
            //setup UI widgets
            window.draw(title);
            tbAlpha.draw(window); tbBeta.draw(window);
            tbGamma.draw(window); tbN.draw(window); tbRadius.draw(window);
            modelToggle.draw(window);
            window.draw(startBtn); window.draw(startText);
        } else {
            //ensure the view matches domain aspect exactly
            if (!viewSet) {
                float windowAspect = (float)window.getSize().x / (float)window.getSize().y;
                float domainAspect = domainW / domainH;
                float viewW = domainW, viewH = domainH;
                if      (windowAspect > domainAspect) { viewW = domainW * (windowAspect / domainAspect); viewH = domainH; }
                else if (windowAspect < domainAspect) { viewW = domainW; viewH = domainH * (domainAspect / windowAspect); }
                view.setCenter(domainW * 0.5f, domainH * 0.5f);
                view.setSize(viewW, viewH);
                window.setView(view);
                viewSet = true;
            }

            //fixed-step simulation timing with accumulator
            float dt = clock.restart().asSeconds();
            acc += (double)dt;

            int S = 0, E = 0, I = 0, R = 0;

            //advance the model in fixed increments
            while (!paused && acc >= stepInterval) {
                tSim += simDT;
                if (usingQuadtree) {
                    QuadtreeSim::Counts c = qsim->step((float)tSim);
                    S = c.S; E = c.E; I = c.I; R = c.R;
                } else {
                    Grid::Counts c = gsim->step((float)tSim);
                    S = c.S; E = c.E; I = c.I; R = c.R;
                }
                acc -= stepInterval;
                if (I <= 0) { paused = true; break; } // auto-pause when outbreak ends
            }

            //render nodes from active model
            if (usingQuadtree) {
                qsim->snapshot(xs, ys, st, ids);
                for (size_t i = 0; i < xs.size(); ++i) {
                    dot.setPosition(xs[i], ys[i]);
                    dot.setFillColor(colorForState(st[i]));
                    window.draw(dot);
                }
                //if paused, recompute counts for HUD from snapshot
                if (paused) {
                    S = 0; E = 0; I = 0; R = 0;
                    for (size_t i = 0; i < st.size(); ++i) {
                        char s = st[i];
                        if (s == 'S') ++S; else if (s == 'E') ++E; else if (s == 'I') ++I; else if (s == 'R') ++R;
                    }
                }
            } else {
                //draw each node at its lattice position
                int Wg = gsim->width(), Hg = gsim->height();
                float dx = (Wg > 1) ? (domainW / (float)(Wg - 1)) : 0.0f;
                float dy = (Hg > 1) ? (domainH / (float)(Hg - 1)) : 0.0f;

                const std::vector<Node>& v = gsim->nodes();
                for (int id = 0; id < (int)v.size(); ++id) {
                    int xg = id % Wg, yg = id / Wg;
                    float x = (Wg == 1) ? (domainW * 0.5f) : (xg * dx);
                    float y = (Hg == 1) ? (domainH * 0.5f) : (yg * dy);
                    dot.setPosition(x, y);
                    dot.setFillColor(colorForState(v[id].getState()));
                    window.draw(dot);
                }
                if (paused) {
                    S = 0; E = 0; I = 0; R = 0;
                    for (size_t i = 0; i < v.size(); ++i) {
                        char s = v[i].getState();
                        if (s == 'S') ++S; else if (s == 'E') ++E; else if (s == 'I') ++I; else if (s == 'R') ++R;
                    }
                }
            }

            //compose string with counts, speed, and hints
            std::ostringstream oss;
            oss << "S: " << S << "   E: " << E << "   I: " << I << "   R: " << R
                << "   |  steps/s: " << (int)stepsPerSecond
                << "   |  " << (paused ? "[PAUSED]" : "[RUNNING]")
                << "   (Space=pause, +/-=speed, Esc=setup)";
            hud.setString(oss.str());

            //draw screen coordinates
            sf::View backup = window.getView();
            window.setView(window.getDefaultView());
            window.draw(hud);
            window.setView(backup);
        }

        window.display();
    }

    //clean up allocated simulators
    if (qsim) delete qsim;
    if (gsim) delete gsim;
    return 0;
}