//
// Created by Wouter Groeneveld on 26/07/18.
//

#ifndef GBA_SPRITE_ENGINE_SPRITE_H
#define GBA_SPRITE_ENGINE_SPRITE_H

#include <libgba-sprite-engine/gba/tonc_types.h>
#include <memory>
#ifdef CODE_COMPILED_AS_PART_OF_TEST
#include <libgba-sprite-engine/gba/tonc_math_stub.h>
#else
#include <libgba-sprite-engine/gba/tonc_math.h>
#endif
#include <libgba-sprite-engine/gbavector.h>

#define COLOR_MODE_16 0
#define COLOR_MODE_256 1
#define GFX_MODE 0
#define MOSAIC_MODE 0
#define AFFINE_FLAG_NONE_SET_YET 0
#define HORIZONTAL_FLIP_FLAG 0
#define VERTICAL_FLIP_FLAG 0

#define FLIP_VERTICAL_CLEAR 0xdfff
#define FLIP_HORIZONTAL_CLEAR 0xefff
#define OAM_TILE_OFFSET_CLEAR 0xfc00
#define OAM_TILE_OFFSET_NEW 0x03ff

enum SpriteSize {
    SIZE_8_8,
    SIZE_16_16,
    SIZE_32_32,
    SIZE_64_64,
    SIZE_16_8,
    SIZE_32_8,
    SIZE_32_16,
    SIZE_64_32,
    SIZE_8_16,
    SIZE_8_32,
    SIZE_16_32,
    SIZE_32_64
};

class SpriteManager;

class Sprite {
private:
    void updateVelocity();
    void updateAnimation();
    void syncVelocity();

protected:
    const void *data;
    int x, y, dx, dy;
    u8 animation_offset;
    u32 priority, w, h, size_bits, shape_bits;
    bool stayWithinBounds;
    u32 imageSize, tileIndex;
    SpriteSize spriteSize;
    u8 animationDelay, numberOfFrames, beginFrame, currentFrame, previousFrame, animationCounter;
    bool animating;
    OBJ_ATTR oam;

    void syncAnimation();
    virtual void syncOam();
    virtual void buildOam(int tileIndex);
    void setAttributesBasedOnSize(SpriteSize size);

public:
    explicit Sprite(const Sprite& other);
    explicit Sprite(const void *imageData, int imageSize, int x, int y, SpriteSize size);
    virtual ~Sprite() {}

    void makeAnimated(int beginFrame, int numberOfFrames, int animationDelay);
    void setBeginFrame(int frame) { this->beginFrame = frame; }
    void animateToFrame(int frame) { this->currentFrame = frame; }
    void animate() { this->animating = true; }
    void stopAnimating() { this->animating = false; }
    void setStayWithinBounds(bool b) { stayWithinBounds = b; }
    void setVelocity(int dx, int dy) {
        this->dx = dx;
        this->dy = dy;
    }
    void update();

    void moveTo(int x, int y);
    void moveTo(VECTOR location);
    bool collidesWith(Sprite &s2);

    void flipVertically(bool flip);
    void flipHorizontally(bool flip);

    u32 getTileIndex() { return tileIndex; }
    VECTOR getPos() { return {x, y}; }
    GBAVector getPosAsVector() { return GBAVector(getPos()); }
    VECTOR getCenter() { return { x + w / 2, y + h / 2 }; }
    VECTOR getVelocity() { return { dx, dy}; }
    u32 getX() { return x; }
    u32 getY() { return y; }
    u32 getDx() { return dx; }
    u32 getDy() { return dy; }
    u32 getWidth() { return w; }
    u32 getHeight() { return h; }
    u32 getAnimationDelay() { return animationDelay; }
    u32 getNumberOfFrames() { return numberOfFrames; }
    u32 getCurrentFrame() { return currentFrame; }
    bool isAnimating() { return animating; };
    bool isOffScreen();

    friend class SpriteManager;
};


#endif //GBA_SPRITE_ENGINE_SPRITE_H
