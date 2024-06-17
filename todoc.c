#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <leif/leif.h>

#define WIN_PADDING 20

static int win_w = 920, win_h = 420;
static LfFont title_font;

int
main(void)
{
  glfwInit();

  GLFWwindow * window = glfwCreateWindow(win_w, win_h, "TodoC", NULL, NULL);

  glfwMakeContextCurrent(window);

  lf_init_glfw(win_w, win_h, window);

  LfTheme theme = lf_get_theme();
  theme.div_props.color = LF_NO_COLOR;
  lf_set_theme(theme);

  title_font = lf_load_font("./font/SpaceMonoNerdFont-Bold.ttf", 30);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    lf_begin();

    lf_div_begin(
      ((vec2s) { WIN_PADDING, WIN_PADDING }),
      ((vec2s) { win_w - WIN_PADDING * 2.0f, win_h - WIN_PADDING * 2.0f }),
      true
    );
    lf_push_font(&title_font);
    lf_text("Hellooo");
    lf_pop_font();

    lf_end();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  lf_free_font(&title_font);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
