try:
	from setuptools import setup

except ImportError:
	from distutils.core import setup

setup(name="Reception",
      version="0.1",
      description="Server Client project for CC3200 (TI) hardware project",
      author="Maxime Clement",
      author_email="maximeclement6@gmail.com",
      url="",
      packages=["serv_rec"],
     )
