// author: Kaan Eraslan

// includes

#ifndef LIGHT_HPP
#define LIGHT_HPP

class LightSource {
public:
  void setIntensity(glm::vec3 intensity);
  void setIntensity(float red, float, green, float blue);
  void setCoeff(glm::vec3 coefficient);
  void setCoeff(float redc, float greenc, float bluec);
  glm::vec3 getIntensity(void);
  glm::vec3 getCoeff(void);
  glm::vec3 getColor(void);
  LightSource(glm::vec3 intensity, glm::vec3 coeff) {
    this->intensity = intensity;
    this->coefficient = coeff;
    this->updateColor();
  }
  LightSource(float red, float redc, float green, float greenc, float blue,
              float bluec) {
    this->intensity = glm::vec3(red, green, blue);
    this->coefficient = glm::vec3(redc, greenc, bluec);
    this->updateColor();
  }
  ~LightSource();

protected:
  glm::vec3 intensity;
  glm::vec3 coefficient;
  glm::vec3 color;

private:
  void updateColor(void);
};

// method declarations

void LightSource::updateColor() {
  // update color of the light source
  this->color.x = this->intensity.x * this->coefficient.x;
  this->color.y = this->intensity.y * this->coefficient.y;
  this->color.z = this->intensity.z * this->coefficient.z;
}

void LightSource::setIntensty(glm::vec3 intensity) {
  /* Set intensity vector to light source
     and update the color afterwards
   */
  this->intensity = intensity;
  this->updateColor();
}
void LightSource::setIntensty(float red, float green, float blue) {
  /* Set intensity values to light source
     and update the color afterwards
   */
  glm::vec3 intensity(red, green, blue);
  this->intensity = intensity;
  this->updateColor();
}

void LightSource::setCoeff(glm::vec3 coeff) {
  /* Set coefficient vector to light source
     and update the color afterwards
   */
  this->coefficient = coeff;
  this->updateColor();
}
void LightSource::setCoeff(float redc, float greenc, float bluec) {
  /* Set intensity values to light source
     and update the color afterwards
   */
  glm::vec3 coeff(redc, greenc, bluec);
  this->coefficient = coeff;
  this->updateColor();
}
glm::vec3 LightSource::getCoeff() { return this->coefficient; }
glm::vec3 LightSource::getColor() { return this->color; }
glm::vec3 LightSource::getIntensity() { return this->intensity; }

class DirectionalLight : public LightSource {
public:
    glm::vec3 direction;
    DirectionalLight(glm::vec3 dir, glm::vec3 intval, glm::vec3 coeff)
    {
        direction = dir;
        coefficient = coeff;
        setIntensty(intval);
    }
    DirectionalLight(float dirx, float diry, float dirz, float intx,
            float inty, float intz, float coeffx, float coeffy,
            float coeffz)
    {
        direction = glm::vec3(dirx, diry, dirz);
        coefficient = glm::vec3(coeffx, coeffy, coeffz);
        setIntensty(intx, inty, intz);

    }
    void setDirection(float dirx, float diry, float dirz)
    {
        direction = glm::vec3(dirx, diry, dirz);
    }
};

class PointLight : public DirectionalLight{
    public:
        glm::vec3 position;
        float attenuationConstant;
        float attenuationLinear;
        float attenuationQuadratic;
};

#endif
